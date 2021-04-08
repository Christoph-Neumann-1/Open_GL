#include <SceneLoader.hpp>
#include <Scene.hpp>
#include <Renderer.hpp>
#include <Log.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Callback.hpp>
#include <Texture.hpp>
#include <Shader.hpp>
#include <Data.hpp>

class S1 final : public Scene
{
    Renderer renderer;
    SceneLoader *loader;

    VertexArray va;

    float data[4][4]{0.0f, 0.0f, 0.0f, 0.0f,
                     800.0f, 0.0f, 1.0f, 0.0f,
                     800.0f, 600.0f, 1.0f, 1.0f,
                     0.0f, 600.0f, 0.0f, 1.0f};

    VertexBuffer vb;

    unsigned int indices[6] = {
        0, 1, 2,
        0, 2, 3};

    IndexBuffer ib;

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), {0, 0, 0});

    Texture tex;
    Shader shader;

    static void OnResize(void *, void *);

public:
    explicit S1(SceneLoader *_load)
        : loader(_load), vb(16 * sizeof(float), nullptr, true), tex(Texture(ROOT_Directory + "/res/Textures/Transparent.png")), shader(Shader(ROOT_Directory + "/res/Shaders/Tex.glsl"))
    {
        VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(2);

        va.AddBuffer(vb, layout);

        OnResize(this, nullptr);

        ib.SetData(indices, 6);

        shader.Bind();
        shader.SetUniform1i("u_Texture", 0);

        renderer.proj = loader->proj;
        renderer.view = view;

        loader->callbackhandler->Register(CallbackHandler::CallbackType::Render, this, OnRender);
        loader->callbackhandler->Register(CallbackHandler::CallbackType::OnWindowResize2, this, OnResize);
    }

    ~S1()
    {
        loader->callbackhandler->RemoveAll(this);
    }

    static void OnRender(void *_this_, void *)
    {
        auto _this = reinterpret_cast<S1 *>(_this_);
        _this->tex.Bind();
        _this->renderer.Draw(_this->va, _this->ib, _this->shader, _this->model);
    }
};

void S1::OnResize(void *_this_, void *)
{
    auto _this = reinterpret_cast<S1 *>(_this_);
    _this->renderer.proj = _this->loader->proj;
    if (_this->loader->window.x / _this->loader->window.y > 16 / 9)
    {
        _this->data[1][0] = 16 * _this->loader->window.y / 9;
        _this->data[2][0] = 16 * _this->loader->window.y / 9;
        _this->data[2][1] = _this->loader->window.y;
        _this->data[3][1] = _this->loader->window.y;
    }
    else
    {
        _this->data[2][1] = 9 * _this->loader->window.x / 16;
        _this->data[3][1] = 9 * _this->loader->window.x / 16;
        _this->data[1][0] = _this->loader->window.x;
        _this->data[2][0] = _this->loader->window.x;
    }
    _this->vb.Bind();
    _this->vb.SetData(4 * 4 * sizeof(float), _this->data);
}

extern "C" Scene *_INIT_(SceneLoader *loader)
{
    return new S1(loader);
}

extern "C" void _EXIT_(Scene *scene)
{
    delete scene;
}
