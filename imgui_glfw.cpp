// ImGui GLFW binding with OpenGL
// https://github.com/ocornut/imgui

#include <stdexcept>
#include <string>

// GLFW
#ifdef HAVE_GLEW
#include <GL/glew.h>
#endif
#include "imgui_glfw.h"
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

// Data
struct ImGui_ImplGlfw
{
    ImGui_ImplGlfw()
    : Window(NULL)
    , Time(0)
    , MouseWheel(0)
    , FontTexture(0)
    , FragmentShader(0)
    , VertexShader(0)
    , Program(0)
    , ModelViewProjection(-1)
    , Position(-1)
    , TexCoord(-1)
    , Color(-1)
    , Texture(-1)
    {
        MousePressed[0] = MousePressed[1] = MousePressed[2] = false;
    }
    GLFWwindow*  Window;
    double       Time;
    bool         MousePressed[3];
    float        MouseWheel;
    GLuint       FontTexture;
    GLuint       FragmentShader;
    GLuint       VertexShader;
    GLuint       Program;
    GLint        ModelViewProjection;
    GLint        Position;
    GLint        TexCoord;
    GLint        Color;
    GLint        Texture;
};

#include <memory>
std::unique_ptr<ImGui_ImplGlfw> g;

const char* kVertexShaderSrc =
   "#ifdef GL_ES\n"
   "precision mediump float;\n"
   "#endif\n"
   "attribute vec2 a_v2Position;                                        \n"
   "attribute vec4 a_v4Color;                                            \n"
   "attribute vec2 a_v2TexCoord;                                        \n"
   "                                                                    \n"
   "uniform mat4 u_m4Projection;                                        \n"
   "                                                                    \n"
   "varying vec4 v_v4Color;                                            \n"
   "varying vec2 v_v2TexCoord;                                            \n"
   "                                                                    \n"
   "void main()                                                        \n"
   "{                                                                    \n"
   "    gl_Position = u_m4Projection * vec4(a_v2Position.xy, 0, 1);        \n"
   "    v_v4Color = a_v4Color;                                            \n"
   "    v_v2TexCoord = a_v2TexCoord;                                    \n"
   "}                                                                    \n";

const char* kFragmentShaderSrc =
   "#ifdef GL_ES\n"
   "precision mediump float;\n"
   "#endif\n"
   "                                                        \n"
   "varying vec4 v_v4Color;                                \n"
   "varying vec2 v_v2TexCoord;                                \n"
   "                                                        \n"
   "uniform sampler2D u_sTexture;                            \n"
   "                                                        \n"
   "void main()                                            \n"
   "{                                                        \n"
   "    vec4 vc = v_v4Color;                                \n"
   "    vec4 tc = texture2D(u_sTexture, v_v2TexCoord);        \n"
   "    gl_FragColor = vc*tc;                                \n"
   "}                                                        \n";

struct PushState {
    PushState(GLenum e) : b(glIsEnabled(e)), e(e) {}
    ~PushState()
    {
        if (b) {
            glEnable(e);
        } else {
            glDisable(e);
        }
    }
    GLboolean b;
    GLenum e;
};

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_ImplGlfw_RenderDrawLists(ImDrawData* draw_data)
{
    if (draw_data->CmdListsCount == 0)
        return;

    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    // glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    PushState blend(GL_BLEND), cull(GL_CULL_FACE), depthTest(GL_DEPTH_TEST),
        scissorTest(GL_SCISSOR_TEST), texture(GL_TEXTURE_2D);
    GLint prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prog);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnableVertexAttribArray(g->Position);
    glEnableVertexAttribArray(g->TexCoord);
    glEnableVertexAttribArray(g->Color);
    glEnable(GL_TEXTURE_2D);


    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context
    glUseProgram(g->Program);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g->Texture, 0);

    // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
    ImGuiIO& io = ImGui::GetIO();
    float fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    glm::mat4 ortho = glm::ortho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f);
    glUniformMatrix4fv(g->ModelViewProjection, 1, GL_FALSE, &ortho[0][0]);

    // Render command lists
    #define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const unsigned char* vtx_buffer = (const unsigned char*)&cmd_list->VtxBuffer.front();
        const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
        glVertexAttribPointer(g->Position, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, pos)));
        glVertexAttribPointer(g->TexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, uv)));
        glVertexAttribPointer(g->Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (void*)(vtx_buffer + OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, GL_UNSIGNED_SHORT, idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }
    #undef OFFSETOF

    glDisableVertexAttribArray(g->Position);
    glDisableVertexAttribArray(g->TexCoord);
    glDisableVertexAttribArray(g->Color);

    // Restore modified state
    glBindTexture(GL_TEXTURE_2D, last_texture);

    // Restore GL state. PushState destructor automatically reset saved value.
    //glPopAttrib();
    glUseProgram(prog);
}

static const char* ImGui_ImplGlfw_GetClipboardText(void *data)
{
    return glfwGetClipboardString(g->Window);
}

static void ImGui_ImplGlfw_SetClipboardText(void *data, const char* text)
{
    glfwSetClipboardString(g->Window, text);
}

void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow*, int button, int action, int /*mods*/)
{
    if (action == GLFW_PRESS && button >= 0 && button < 3) {
        g->MousePressed[button] = true;
    }
}

void ImGui_ImplGlfw_ScrollCallback(GLFWwindow*, double /*xoffset*/, double yoffset)
{
    g->MouseWheel += (float)yoffset; // Use fractional mouse wheel, 1.0 unit 5 lines.
}

void ImGui_ImplGlFw_KeyCallback(GLFWwindow*, int key, int, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)
        io.KeysDown[key] = true;
    if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;

    (void)mods; // Modifiers are not reliable across systems
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
}

void ImGui_ImplGlfw_CharCallback(GLFWwindow*, unsigned int c)
{
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

bool ImGui_ImplGlfw_CreateDeviceObjects()
{
    ImGuiIO& io = ImGui::GetIO();

    // Build texture
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    // Create texture
    glGenTextures(1, &g->FontTexture);
    glBindTexture(GL_TEXTURE_2D, g->FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)g->FontTexture;

    // Cleanup (don't clear the input data if you want to append new fonts later)
    io.Fonts->ClearInputData();
    io.Fonts->ClearTexData();

    return true;
}

void    ImGui_ImplGlfw_InvalidateDeviceObjects()
{
    if (g->FontTexture) {
        glDeleteTextures(1, &g->FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g->FontTexture = 0;
    }
}


GLuint ImGui_ImplGlfw_CreateShader(GLenum shaderType, const char* shaderSrc, GLuint& shader)
{
    GLint compiled;
    shader = glCreateShader(shaderType);
    if (shader == 0) {
        throw std::runtime_error("Creating shader failed");
    }
    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        std::string msg;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 1) {
            msg.resize(infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, &msg[0]);
        }
        glDeleteShader(shader);
        shader = 0;
        throw std::runtime_error(msg);
    }
    return shader;
}

void ImGui_ImplGlfw_CreateProgram()
{
    g->Program = glCreateProgram();
    if (g->Program == 0) {
        throw std::runtime_error("Failed to create GL Program");
    }

    glAttachShader(g->Program, g->VertexShader);
    glAttachShader(g->Program, g->FragmentShader);

    glLinkProgram(g->Program);

    GLint linked;
    glGetProgramiv(g->Program, GL_LINK_STATUS, &linked);
    if(!linked) {
        GLint infoLen = 0;
        std::string msg;
        glGetProgramiv(g->Program, GL_INFO_LOG_LENGTH, &infoLen);
        if(infoLen > 1) {
            msg.resize(infoLen);
            glGetProgramInfoLog(g->Program, infoLen, NULL, &msg[0]);
        }
        glDeleteProgram(g->Program);
        g->Program = 0;
        throw std::runtime_error(msg);
    }

    g->Position = glGetAttribLocation(g->Program, "a_v2Position");
    g->Color = glGetAttribLocation(g->Program, "a_v4Color");
    g->TexCoord = glGetAttribLocation(g->Program, "a_v2TexCoord");

    g->ModelViewProjection = glGetUniformLocation(g->Program, "u_m4Projection");
    g->Texture = glGetUniformLocation(g->Program, "u_sTexture");
}

void ImGui_ImplGlfw_InitGL()
{
#ifdef HAVE_GLEW
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        throw std::runtime_error("imgui-glfw: Failed to initialize GLEW");
    }
    if (!GLEW_VERSION_2_0) {
        throw std::runtime_error("imgui-glfw: OpenGL 2.0 not supported");
    }
#endif
    ImGui_ImplGlfw_CreateShader(GL_VERTEX_SHADER, kVertexShaderSrc, g->VertexShader);
    ImGui_ImplGlfw_CreateShader(GL_FRAGMENT_SHADER, kFragmentShaderSrc, g->FragmentShader);
    ImGui_ImplGlfw_CreateProgram();
}

void ImGui_ImplGlfw_ReleaseGL()
{
   glDeleteProgram(g->Program);
   glDeleteShader(g->FragmentShader);
   glDeleteShader(g->VertexShader);
}

bool ImGui_ImplGlfw_Init(GLFWwindow* window, bool install_callbacks)
{
    g.reset(new ImGui_ImplGlfw);
    g->Window = window;
    ImGui_ImplGlfw_InitGL();

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                 // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.RenderDrawListsFn = ImGui_ImplGlfw_RenderDrawLists;
    io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
#ifdef _MSC_VER
    io.ImeWindowHandle = glfwGetWin32Window(g->Window);
#endif

    if (install_callbacks) {
        glfwSetMouseButtonCallback(window, ImGui_ImplGlfw_MouseButtonCallback);
        glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
        glfwSetKeyCallback(window, ImGui_ImplGlFw_KeyCallback);
        glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);
    }
    return true;
}

void ImGui_ImplGlfw_Shutdown()
{
    ImGui_ImplGlfw_ReleaseGL();
    ImGui_ImplGlfw_InvalidateDeviceObjects();
    ImGui::Shutdown();
}

void ImGui_ImplGlfw_NewFrame()
{
    if (!g->FontTexture) {
        ImGui_ImplGlfw_CreateDeviceObjects();
    }
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(g->Window, &w, &h);
    glfwGetFramebufferSize(g->Window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)display_w, (float)display_h);

    // Setup time step
    double current_time =  glfwGetTime();
    io.DeltaTime = g->Time > 0.0 ? (float)(current_time - g->Time) : (float)(1.0f/60.0f);
    g->Time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    if (glfwGetWindowAttrib(g->Window, GLFW_FOCUSED)) {
        double mouse_x, mouse_y;
        glfwGetCursorPos(g->Window, &mouse_x, &mouse_y);
        mouse_x *= (float)display_w / w;                        // Convert mouse coordinates to pixels
        mouse_y *= (float)display_h / h;
        io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    } else {
        io.MousePos = ImVec2(-1,-1);
    }

    for (int i = 0; i < 3; i++) {
        io.MouseDown[i] = g->MousePressed[i] || glfwGetMouseButton(g->Window, i) != 0;    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        g->MousePressed[i] = false;
    }

    io.MouseWheel = g->MouseWheel;
    g->MouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    glfwSetInputMode(g->Window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);

    // Start the frame
    ImGui::NewFrame();
}
