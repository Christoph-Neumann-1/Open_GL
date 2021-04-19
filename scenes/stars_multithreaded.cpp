#include <SceneLoader.hpp>
#include <Scene.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Callback.hpp>
#include <Shader.hpp>
#include <Data.hpp>
#include <Flycam.hpp>
#include <Model_Instanced.hpp>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include <random>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <array>

class PhysicsThread
{

    static void ThreadLoop(PhysicsThread *thread);

    bool exit = false;
    bool dispatch = false;
    std::atomic_bool ready = true;

    std::function<void(int, int)> function;
    std::condition_variable cv;
    std::mutex mutex;
    int param1, param2;

    std::thread t;

public:
    PhysicsThread() : t(ThreadLoop, this) {}
    ~PhysicsThread()
    {

        join();
        exit = true;
        dispatch = true;
        cv.notify_one();
        t.join();
    }

    void Dispatch(std::function<void(int, int)> function, int param1, int param2)
    {
        std::unique_lock<std::mutex> lk(mutex);
        cv.wait(lk, [&]() { return ready.load(); });

        this->function = function;
        this->param1 = param1;
        this->param2 = param2;
        ready = false;
        dispatch = true;
        lk.unlock();
        cv.notify_one();
    }
    void join()
    {
        std::unique_lock<std::mutex> lk(mutex);
        while (!ready.load())
            cv.wait_for(lk, std::chrono::milliseconds(1), [&]() { return ready.load(); });
    }
};

void PhysicsThread::ThreadLoop(PhysicsThread *thread)
{
    while (1)
    {
        std::unique_lock<std::mutex> lk(thread->mutex);
        thread->cv.wait(lk, [&]() { return thread->dispatch; });

        if (thread->exit)
            return;

        thread->function(thread->param1, thread->param2);

        thread->ready = true;
        thread->dispatch = false;
        thread->function = nullptr;
        lk.unlock();
        thread->cv.notify_one();
    }
}

template <u_int thread_count>
class Threadgroup
{
    std::array<PhysicsThread, thread_count> threads;

public:
    const u_int n_threads = thread_count;

    void Dispatch(std::function<void(int, int)> function, std::array<std::pair<int, int>, thread_count> parameters)
    {
        for (u_int i = 0; i < n_threads; i++)
        {
            threads[i].Dispatch(function, parameters[i].first, parameters[i].second);
        }
    }
    void join()
    {
        for (auto &thread : threads)
            thread.join();
    }
};

struct Star
{
    glm::vec3 pos;
    glm::vec3 velocity;
    float scale;
};

const float spawnrange = 160;
const float spawnv = 18;
const float startscale = 1;
const u_int nStars = 2400;
const float G = 13;
const int physicsfreq = 120;
const u_int nThreads = 6;

class StarSim final : public Scene
{
    SceneLoader *loader;
    Model star;
    unsigned int Transforms;
    std::vector<Star> stars;
    Shader shader;

    Camera3D camera;
    Flycam camcontrol;
    glm::mat4 proj = glm::perspective(glm::radians(65.0f), (float)loader->window.x / (float)loader->window.y, 0.1f, 1500.0f);
    std::vector<std::pair<int, int>> collisions;
    std::vector<int> deleted_stars;

    std::mutex modify_stars;
    Threadgroup<nThreads> physics_threads;
    std::thread *physics_thread;
    std::atomic_bool kill_thread = false;

    void OnUpdate()
    {
        std::mutex collision_lock;
        float dt = 1.0 / physicsfreq;

        auto ComputeForcesAndCollide = [&](int startindex, int endindex) {
            for (int i = startindex; i < endindex; i++)
            {
                float volume1 = 4 / 3 * M_PI * powf(stars[i].scale, 3);
                for (int j = 0; j < (long int)stars.size(); j++)
                {
                    if (i == j)
                        continue;
                    float volume2 = 4 / 3 * M_PI * powf(stars[j].scale, 3);
                    glm::vec3 vec = stars[j].pos - stars[i].pos;
                    float dist = glm::length(vec);
                    float force = volume1 * volume2 / powf(dist, 2) * G * dt;
                    stars[i].velocity += force / volume1 * glm::normalize(vec);
                    if (dist <= stars[i].scale + stars[j].scale)
                    {
                        std::lock_guard<std::mutex> lk(collision_lock);
                        bool has_collided = false;
                        for (auto x : collisions)
                        {
                            if (i==x.first || i==x.second || j==x.first || j==x.second)
                            {
                                has_collided = true;
                                break;
                            }
                            
                        }
                        if(!has_collided)
                        collisions.emplace_back(i, j);
                    }
                }
            }
        };
        auto UpdatePositions = [&](int startindex, int endindex) {
            for (int i = startindex; i < endindex; i++)
            {
                stars[i].pos += stars[i].velocity * dt;
            }
        };
        std::array<std::pair<int, int>, nThreads> ranges;

        auto ComputeRanges = [&]() {
            int size = stars.size() / nThreads;
            for (u_int i = 0; i < nThreads; i++)
            {
                ranges[i].first = i * size;
                ranges[i].second = (i + 1) * size;
            }
            ranges[nThreads - 1].second = stars.size();
        };

        ComputeRanges();

        while (!kill_thread)
        {

            std::chrono::time_point<std::chrono::high_resolution_clock> begin = std::chrono::high_resolution_clock::now();

            physics_threads.Dispatch(ComputeForcesAndCollide, ranges);
            physics_threads.join();
            physics_threads.Dispatch(UpdatePositions, ranges);
            physics_threads.join();

            for (auto collision : collisions)
            {
                float v1 = (4.0 / 3.0) * M_PI * powf(stars[collision.first].scale, 3);
                float v2 = (4.0 / 3.0) * M_PI * powf(stars[collision.second].scale, 3);
                float combined_scale = powf(3 * (v1 + v2) / (4 * M_PI), 1.0 / 3.0);
                glm::vec3 centerofmass = (stars[collision.first].pos * v1 + stars[collision.second].pos * v2) / (v1 + v2);
                glm::vec3 average_velocity = (stars[collision.first].velocity * v1 + stars[collision.second].velocity * v2) / (v1 + v2);

                stars[collision.first] = {centerofmass, average_velocity, combined_scale};
                deleted_stars.emplace_back(collision.second);
            }
            std::sort(deleted_stars.begin(), deleted_stars.end(), std::greater<int>());

            std::unique_lock<std::mutex> lk(modify_stars);
            for (int i : deleted_stars)
            {
                stars.erase(stars.begin() + i);
            }
            lk.unlock();

            if (deleted_stars.size() != 0)
                ComputeRanges();

            deleted_stars.clear();
            collisions.clear();

            int64_t time = duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - begin).count();
            int64_t deltatime = powf(10, 9) / physicsfreq;
            int64_t sleeptime = deltatime - time;
            if (sleeptime < 0)
                continue;
            std::this_thread::sleep_for(std::chrono::nanoseconds(sleeptime));
        }
    }

    void OnRender()
    {
        camcontrol.Update(loader->callbackhandler->deltatime_update);
        shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());
        std::lock_guard<std::mutex> lk(modify_stars);
        glBindBuffer(GL_ARRAY_BUFFER, Transforms);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Star) * stars.size(), &stars[0]);
        star.Draw(shader, stars.size());
    }

public:
    StarSim(SceneLoader *loader);

    ~StarSim()
    {
        loader->callbackhandler->RemoveAll(this);
        kill_thread = true;
        physics_thread->join();
    }
};

StarSim::StarSim(SceneLoader *loader)
    : loader(loader), star(ROOT_Directory + "/res/Models/star.obj"), shader(ROOT_Directory + "/res/Shaders/Star.glsl"), camera({0, 0, 50}),
      camcontrol(&camera, loader->window, 150, 120, 0.11)
{
    glGenBuffers(1, &Transforms);
    InstanceBufferLayout layout;
    layout.stride = sizeof(Star);
    layout.attributes.emplace_back(GL_FLOAT, 3, (void *)0);
    layout.attributes.emplace_back(GL_FLOAT, 3, (void *)offsetof(Star, velocity));
    layout.attributes.emplace_back(GL_FLOAT, 1, (void *)offsetof(Star, scale));
    star.AddInstanceBuffer(layout, Transforms);
    loader->callbackhandler->Register(CallbackHandler::CallbackType::Render, this, [](void *x, void *) { reinterpret_cast<StarSim *>(x)->OnRender(); });
    glBindBuffer(GL_ARRAY_BUFFER, Transforms);

    stars.reserve(nStars);
    srand(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
    for (u_int i = 0; i < nStars; i++)
    {
        glm::vec3 pos = {((float)rand() / (RAND_MAX / 2) - 1) * spawnrange, ((float)rand() / (RAND_MAX / 2) - 1) * spawnrange, ((float)rand() / (RAND_MAX / 2) - 1) * spawnrange};
        glm::vec3 vel = {((float)rand() / (RAND_MAX / 2) - 1) * spawnv, ((float)rand() / (RAND_MAX / 2) - 1) * spawnv, ((float)rand() / (RAND_MAX / 2) - 1) * spawnv};
        stars.emplace_back(pos, vel, ((float)rand() / RAND_MAX + 0.5) * startscale);
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(Star) * nStars, &stars[0], GL_DYNAMIC_DRAW);

    flags["hide_menu"] = true;
    physics_thread = new std::thread([&]() { OnUpdate(); });
}

extern "C"
{

    Scene *_INIT_(SceneLoader *loader)
    {
        return new StarSim(loader);
    }

    void _EXIT_(Scene *scene)
    {
        delete scene;
    }
}