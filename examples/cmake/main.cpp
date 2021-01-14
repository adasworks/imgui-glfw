#include <stdexcept>
#include <cstdio>

#include <GLFW/glfw3.h>
#include "imgui_glfw.h"
#include "implot/implot.h"

#define THROW_IF(x,msg) { if ((x)) { throw std::runtime_error(msg); } }

int main(int argc, char* argv[])
{
    try {
        THROW_IF(glfwInit() != GL_TRUE, "Failed to init GLFW");
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);

        // This is required with VirtualBox: "X Error of failed request:
        // BadMatch (invalid parameter attributes)"
        glfwWindowHint(GLFW_ALPHA_BITS, 0);

        GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui + GLFW3", 0, 0);
        THROW_IF(window == 0, "Failed to create window");
        glfwMakeContextCurrent(window);

        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImPlot::SetImGuiContext(ImGui::GetCurrentContext());
        ImGui_ImplGlfw_Init(window, true);

        printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
        printf("GL_VERSION:  %s\n", glGetString(GL_VERSION));
        printf("GL_VENDOR:   %s\n", glGetString(GL_VENDOR));

        // Load Fonts
        // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
        //ImGuiIO& io = ImGui::GetIO();
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
        //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
        //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

        bool show_test_window = true;
        bool show_plot_test_window = true;
        bool show_another_window = false;
        bool show_tab_window = true;
        ImVec4 clear_color = ImColor(114, 144, 154);

        // Main loop
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // 1. Show a simple window
            // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
            {
                static float f = 0.0f;
                ImGui::Text("Hello, world!");
                ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
                ImGui::ColorEdit3("clear color", (float*)&clear_color);
                if (ImGui::Button("Test Window")) show_test_window ^= 1;
                if (ImGui::Button("ImPlot Test Window")) show_plot_test_window ^= 1;
                if (ImGui::Button("Another Window")) show_another_window ^= 1;
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            }

            // 2. Show another simple window, this time using an explicit Begin/End pair
            if (show_another_window)
            {
                ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
                ImGui::Begin("Another Window", &show_another_window);
                ImGui::Text("Hello");
                ImGui::End();
            }

            // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow()
            if (show_test_window)
            {
                ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
                ImGui::ShowDemoWindow(&show_test_window);
            }

            if (show_plot_test_window)
            {
                ImPlot::ShowDemoWindow(&show_plot_test_window);
            }

            if (show_tab_window)
            {
                ImGui::Begin("ImGui Tabs");
                ImGui::BeginTabBar("Settings#left_tabs_bar", ImGuiTabBarFlags_None);
                if (ImGui::BeginTabItem("General")) {
                    static bool fullscreen = false;
                    if (ImGui::Checkbox("Fullscreen Mode", &fullscreen)) {
                    }
                    static bool enableMultisampling = false;
                    if (ImGui::Checkbox("Enable Multisampling", &enableMultisampling)) {

                    }

                    static int multisampleCount = 4;
                    if (ImGui::SliderInt("MSAA Count", &multisampleCount, 1, 9)) {

                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("GUI")) {
                    ImGui::Text("Tab 2");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Tab Name")) {
                    ImGui::Text("Tab 3");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Tab Name")) {
                    ImGui::Text("Tab 4");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();

                ImGui::BeginTabBar("Empty", ImGuiTabBarFlags_None);
                ImGui::EndTabBar();

                ImGui::Dummy(ImVec2(0, 20));

                ImGui::BeginTabBar("#Additional Parameters", ImGuiTabBarFlags_None);
                float value = 0.0f;
                if (ImGui::BeginTabItem("Tab Name2")) {
                    ImGui::SliderFloat("Slider", &value, 0, 1.0f);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Tab Name3")) {
                    ImGui::Text("Tab 2");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Tab Name4")) {
                    ImGui::Text("Tab 3");
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Tab Name5")) {
                    ImGui::Text("Tab 4");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
                ImGui::End();
            }

            // Rendering
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui::Render();
            ImGui_ImplGlfw_RenderDrawLists(ImGui::GetDrawData());
            glfwSwapBuffers(window);
        }

	ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    } catch (const std::exception& e) {
        printf("Exception caught: %s\n", e.what());
    }
    return 0;
}
