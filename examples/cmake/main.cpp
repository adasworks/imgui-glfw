#include <stdexcept>
#include <cstdio>

#include "imgui_glfw.h"

#define THROW_IF(x,msg) { if ((x)) { throw std::runtime_error(msg); } }

int main(int argc, char* argv[])
{
    try {
        THROW_IF(glfwInit() != GL_TRUE, "Failed to init GLFW");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#ifdef GLFW_USE_GLESV2
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#else
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
#endif

        // This is required with VirtualBox: "X Error of failed request:
        // BadMatch (invalid parameter attributes)"
        glfwWindowHint(GLFW_ALPHA_BITS, 0);

        GLFWwindow* wnd = glfwCreateWindow(320, 240, "ImGui + GLFW3", 0, 0);
        THROW_IF(wnd == 0, "Failed to create window");
        glfwMakeContextCurrent(wnd);
        ImGui_ImplGlfw_Init(wnd, true);

        printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
        printf("GL_VERSION:  %s\n", glGetString(GL_VERSION));
        printf("GL_VENDOR:   %s\n", glGetString(GL_VENDOR));

        bool show_test_window = true;
        while (!glfwWindowShouldClose(wnd)) {
            int w = 0, h = 0;
            glfwGetFramebufferSize(wnd, &w, &h);
            glViewport(0,0,w,h);
            glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

            ImGui_ImplGlfw_NewFrame();

            if (show_test_window) {
                ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
                ImGui::ShowTestWindow(&show_test_window);
            }
            ImGui::Render();

            glfwSwapBuffers(wnd);
            glfwPollEvents();
            if (glfwGetKey(wnd, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
                glfwGetKey(wnd, GLFW_KEY_Q) == GLFW_PRESS) {
                glfwSetWindowShouldClose(wnd, true);
            }
        }

        ImGui_ImplGlfw_Shutdown();

        glfwDestroyWindow(wnd);
        glfwTerminate();
    } catch (const std::exception& e) {
        printf("Exception caught: %s\n", e.what());
    }
    return 0;
}
