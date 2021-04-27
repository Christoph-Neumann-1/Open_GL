#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <Scene.hpp>
#include <Callback.hpp>
#include <DeltaTime.hpp>
#include <Logger.hpp>
#include <filesystem>
#include <ErrorOutput.hpp>
#include <imgui/imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <thread>
#include <condition_variable>
#include <pthread.h>

#include <ModuleLoader.hpp>

std::string ROOT_Directory;

using namespace GL;
using cbt = CallbackType;

void UpdateLoop(CallbackHandler &cbh, TimeInfo &ti, std::atomic_bool &close, std::condition_variable &cv, float frequency)
{
    Logger log;
    auto &updatecb = cbh.GetList(cbt::Update);
    auto &preupdatecb = cbh.GetList(cbt::PreUpdate);
    auto &postupdatecb = cbh.GetList(cbt::PostUpdate);

    ti.SetUpdateInterval(1 / frequency);

    std::chrono::nanoseconds Interval((int)(powf(10, 9) / frequency));
    std::chrono::nanoseconds behind(0);

#ifdef DEBUG_LOG
    log << "Update thread ready starting callbacks.";
    log.print();
#endif

    while (!close)
    {
        auto begin = std::chrono::high_resolution_clock::now();

        preupdatecb();
        updatecb();
        postupdatecb();

        auto end = std::chrono::high_resolution_clock::now();
        auto time = Interval - (end - begin);
        if (time < std::chrono::nanoseconds(0))
            behind = time;
        else
        {
            PreciseSleep(time + behind);
        }
    }
#ifdef DEBUG_LOG
    log << "Update thread starting cleanup.";
    log.print();
#endif

#ifdef DEBUG_LOG
    log << "Update thread finished cleanup.";
    log.print();
#endif
    close = false;
    cv.notify_one();
}

int main(int argc, char **argv)
{
    Logger log;

    ROOT_Directory = argc > 1 ? argv[1] : std::filesystem::path(std::string(argv[0])).parent_path().string();
    if (!std::filesystem::exists(ROOT_Directory + "/res"))
        ROOT_Directory = std::filesystem::path(ROOT_Directory).parent_path().string(); // Needed to run it from build directory.



    {
        CallbackHandler cbh;
        TimeInfo timeinfo(cbh);

#pragma region

        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
        {
            log << "Failed to initialize glfw";
            log.print();
            return 1;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_RESIZABLE, true);
        glfwWindowHint(GLFW_SAMPLES, 4);

        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow *_window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
        if (!_window)
        {
            log << "Failed to create window";
            log.print();
            return 1;
        }

        Window window(_window, cbh.GetList(cbt::OnWindowResize));

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            log << "Failed to initialize glad.";
            log.print();
            return 1;
        }

        glViewport(0, 0, window.GetWidth(), window.GetHeigth());

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        // glEnable(GL_CULL_FACE); Still not compatible with everything
        glDebugMessageCallback(ErrorCallback, 0);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        {
            int major, minor;
            glGetIntegerv(GL_MAJOR_VERSION, &major);
            glGetIntegerv(GL_MINOR_VERSION, &minor);
            log << "GL Version: " << major << '.' << minor;
            log << " Vendor: " << glGetString(GL_VENDOR);
            log << " Renderer name: " << glGetString(GL_RENDERER);
            log.print();
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGui::StyleColorsDark();
#pragma endregion


        SceneLoader loader(window, cbh, timeinfo);

        auto &rendercb = cbh.GetList(cbt::Render);
        auto &prerendercb = cbh.GetList(cbt::PreRender);
        auto &postrendercb = cbh.GetList(cbt::PostRender);

        auto &imguirendercb = cbh.GetList(cbt::ImGuiRender);

        std::mutex mutex;
        std::condition_variable cv;
        std::atomic_bool should_close = false;

        std::thread UpdateThread(std::ref(UpdateLoop), std::ref(cbh), std::ref(timeinfo), std::ref(should_close), std::ref(cv), 100.0f);

        while (!glfwWindowShouldClose(window))
        {
            glClearColor(0.2, 0.2, 0.2, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            prerendercb();
            rendercb();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            imguirendercb();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            postrendercb();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        should_close = true;
        std::unique_lock lk(mutex);
        if (cv.wait_for(lk, std::chrono::milliseconds(100), [&]() { return !should_close; }))
            UpdateThread.join();
        else
        {
            log << "Update thread took to long to exit. Terminating now.";
            log.print();
            UpdateThread.detach();
            pthread_cancel(UpdateThread.native_handle());
        }

        log << "Window closed with Flag " << glfwWindowShouldClose(window);
        log.print();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        loader.Terminate();
    }
    glfwTerminate();
}