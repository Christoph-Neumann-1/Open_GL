#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Scene.hpp>
#include <Callback.hpp>
#include <Time.hpp>
#include <Logger.hpp>
#include <filesystem>
#include <ErrorOutput.hpp>
#include <imgui/imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <thread>
#include <condition_variable>
#include <Data.hpp>
#include <Input.hpp>

using namespace GL;
using cbt = CallbackType;

///This function schedules a function to be called once both threads are ready. This may be useful for unloading or loading scenes.
static void AddToSync(std::mutex &mutex, std::vector<std::function<void()>> &syncs, const std::function<void()> &func)
{
    std::scoped_lock lk(mutex);
    syncs.push_back(func);
}
/**
 * @brief This is the update thread intended for all operations not directly related to rendering.
 * 
 * @param cbh All relevant callbacks
 * @param ti Just for the update interval, this is not passed as a value as it may change.
 * @param close This is used to signal the thread to close, it also informs the main aka rendering thread that this therad is done.
 * @param cv //Notifies the main thread that this thread is done.
 * @param should_sync //True if this thread should wait for the synchronized fucntions to be called.
 * @param is_synced //Communicates whether the thread is waiting.
 */
static void UpdateLoop(CallbackHandler &cbh, TimeInfo &ti, std::atomic_bool &close, std::condition_variable &cv,
                       std::atomic_bool &should_sync, std::atomic_bool &is_synced)
{
    Logger log;
    auto &updatecb = cbh.GetList(cbt::Update);
    auto &preupdatecb = cbh.GetList(cbt::PreUpdate);
    auto &postupdatecb = cbh.GetList(cbt::PostUpdate);
    const float &interval = ti.UpdateInterval();

#ifdef DEBUG_LOG
    log << "Update thread ready starting callbacks.";
    log.print();
#endif

//This code counts how many times loop is run every second. I use this for proffiling.
#define UPDATE_COUNT
#ifdef UPDATE_COUNT
    int second = 0;
    int count = 0;
    auto fs = std::chrono::high_resolution_clock::now();
#endif

    while (!close)
    {
        //Determine how long each iteration should take.
        std::chrono::nanoseconds Interval((int)(powf(10, 9) * interval));
        auto begin = std::chrono::high_resolution_clock::now();

#ifdef UPDATE_COUNT

        if (floor((begin - fs).count() / powf(10, 9)) == second)
        {
            count++;
        }
        else
        {
            log(count);
            second = floor((begin - fs).count() / powf(10, 9));
            count = 0;
        }
#endif

        preupdatecb();
        updatecb();
        postupdatecb();

        if (should_sync)
        {
            is_synced = true;
            while (should_sync)
                ;
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto time = Interval - (end - begin);
        PreciseSleep(time);
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


    //Determine the directory in which all the resources, such as shader and textures are found.
    //If a command line argument is passed, use that as the directory.
    //Otherwise it is checked if there is a res subdrive in the directory of the executable.
    //If not, check the parent directory. This is necessary for debugging as the exectable is in the build directory.
    ROOT_Directory = argc > 1 ? argv[1] : std::filesystem::path(std::string(argv[0])).parent_path().string();
    if (!std::filesystem::exists(ROOT_Directory + "/res") && std::filesystem::exists(ROOT_Directory + "/../res"))
        ROOT_Directory = std::filesystem::path(ROOT_Directory + "/.."); // Needed to run it from build directory.

    {
        std::mutex mutex;
        std::vector<std::function<void()>> syncfunctions;
        CallbackHandler cbh(std::bind(AddToSync, std::ref(mutex), std::ref(syncfunctions), std::placeholders::_1));
        TimeInfo timeinfo(cbh);

#pragma region

        glfwSetErrorCallback(glfw_error_callback);
        //There is no point trying to continue if the GLFW initialization fails.
        if (!glfwInit())
        {
            log << "Failed to initialize glfw";
            log.print();
            return 1;
        }

        //I chose this version because it worked in virtal box.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_RESIZABLE, true);
        glfwWindowHint(GLFW_SAMPLES, 8);

        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        //By default, the window is full screen.
        auto vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        GLFWwindow *_window = glfwCreateWindow(vidmode->width, vidmode->height, "OpenGL", glfwGetPrimaryMonitor(), NULL);
        if (!_window)
        {
            log << "Failed to create window";
            log.print();
            return 1;
        }

        Window window(_window, cbh.GetList(cbt::OnWindowResize));
        InputHandler handler(window);
        window.inputptr = &handler;

        //Turns on VSync.
        glfwMakeContextCurrent(window);
        glfwWindowHint(GLFW_REFRESH_RATE, vidmode->refreshRate);
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
        glEnable(GL_CULL_FACE);
        glDebugMessageCallback(ErrorCallback, 0);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        //Print version info and driver name.
        {
            int major, minor;
            glGetIntegerv(GL_MAJOR_VERSION, &major);
            glGetIntegerv(GL_MINOR_VERSION, &minor);
            log << "GL Version: " << major << '.' << minor;
            log << " Vendor: " << glGetString(GL_VENDOR);
            log << " Renderer name: " << glGetString(GL_RENDERER);
            log.print();
        }

        //Imgui boilerplate.
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGui::StyleColorsDark();

#pragma endregion

        SceneLoader loader(window, cbh, timeinfo);

        //If the menu closes, stop the program.
        loader.SetUnloadCb([&](SceneLoader *)
                           {
                               glfwSetWindowShouldClose(window, 2);
                               return true;
                           });

        //The menu allows for the selection of a scene by default it stays visible, but can be hidden by the scene.
        loader.Load(ROOT_Directory + "/scenes/bin/Menu.scene");

        auto &rendercb = cbh.GetList(cbt::Render);
        auto &prerendercb = cbh.GetList(cbt::PreRender);
        auto &postrendercb = cbh.GetList(cbt::PostRender);

        auto &imguirendercb = cbh.GetList(cbt::ImGuiRender);

        std::condition_variable cv;
        std::atomic_bool should_close = false;
        std::atomic_bool should_sync = false;
        std::atomic_bool is_synced = false;

        timeinfo.SetUpdateInterval();

        //Start the thread, now that everything is initialized.
        std::thread UpdateThread(std::ref(UpdateLoop), std::ref(cbh), std::ref(timeinfo), std::ref(should_close),
                                 std::ref(cv), std::ref(should_sync), std::ref(is_synced));

        //The value will be non-zero if the window should close. The value can tell you how the window was closed.
        while (!glfwWindowShouldClose(window))
        {
            //Blue background.
            glClearColor(21 / 255.0, 132 / 255.0, 201 / 255.0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            //Draw normal stuff.
            prerendercb();
            rendercb();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            //Interface
            imguirendercb();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            postrendercb();

            //Check if the threads should synchronize, if so wait for the update thread to finish.
            {
                std::unique_lock lk(mutex);
                if (syncfunctions.size())
                {
                    should_sync = true;
                    while (!is_synced)
                        ;
                    for (auto &func : syncfunctions)
                        func();
                    syncfunctions.clear();
                    is_synced = false;
                    should_sync = false;
                }
            }
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        //Try stopping the update thread, if it takes too long kill it.
        should_close = true;
        std::unique_lock lk(mutex);
        if (cv.wait_for(lk, std::chrono::milliseconds(100), [&]()
                        { return !should_close; }))
            UpdateThread.join();
        else
        {
            log << "Update thread took to long to exit. Terminating now.";
            log.print();
            UpdateThread.detach();
            pthread_cancel(UpdateThread.native_handle());
        }

        log << "Window closed with Flag " << glfwWindowShouldClose(window);
        switch (glfwWindowShouldClose(window))
        {
        case 1:
            log << " (Window closed normally)";
            break;
            //I just used the next number, have to find a better way to do this.
        case 2:
            log << " (Window closed by exit button)";
            break;
        }
        log.print();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    //This happens here because the destructors of the render classes will complain if the context no longer exists.
    glfwTerminate();

    //Not necessary, but it's nice to have.
    return 0;
}
