/**
 * @file Application.cpp
 * @brief Entry Point of the application
 * Contains Mainloop and most of the setup.
 * @link md_DocSrc_MCode Source Code @endlink
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <Window.hpp>
#include <ErrorHandler.hpp>
#include <Log.hpp>
#include <Data.hpp>
#include <SceneLoader.hpp>
#include <Scene.hpp>
#include <imgui/imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static Logger logger;

int main()
{
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
        {
            return 1;
        }
        Window window(1000, 800, "GL_Test", 4, 6);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            logger << "Failed to initialize glad.";
            logger.print();
            return 1;
        }

        glViewport(0, 0, window.x, window.y);

        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(ErrorCallback, 0);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        CallbackHandler cb;
        SceneLoader menu(window, &cb);
        menu.Load((ROOT_Directory + "/scenes/bin/menu.so").c_str());

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGui::StyleColorsDark();

        int fbx;
        int fby;

        while (!window.CloseFlag())
        {
            glClear(GL_COLOR_BUFFER_BIT);
            glfwGetFramebufferSize(window, &fbx, &fby);

            if (fbx != window.x || fby != window.y)
            {
                window.x = fbx;
                window.y = fby;
                glViewport(0, 0, fbx, fby);
                cb.Call(CallbackHandler::CallbackType::OnWindowResize);
                cb.Call(CallbackHandler::CallbackType::OnWindowResize2);
            }
            if (!window.paused)
            {

                cb.Call(CallbackHandler::CallbackType::Update);
            }
            cb.Call(CallbackHandler::CallbackType::PreRender);

            cb.Call(CallbackHandler::CallbackType::Render);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            cb.Call(CallbackHandler::CallbackType::ImGuiRender);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        logger << "Window closed with Flag " << window.CloseFlag();
        logger.print();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    glfwTerminate();
}