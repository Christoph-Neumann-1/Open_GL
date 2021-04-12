#include <SceneLoader.hpp>
#include <Scene.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Callback.hpp>
#include <Texture.hpp>
#include <Shader.hpp>
#include <Data.hpp>
#include <glm/gtx/quaternion.hpp>
#include <Camera3D.hpp>
#include <Flycam.hpp>

class S1 final : public Scene
{
    SceneLoader *loader;

    VertexArray va;
    float vertices[5 * 36] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

    glm::vec3 offsets[3]={{1.5,0,0},{0.6,0.7,0},{0.3,0.2,3}};
    u_int offBuff;

    VertexBuffer vb;

    Camera3D camera;
    Flycam fcam;

    glm::mat4 view;

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3{0, 0, 0});
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), {1.0f, 1.0f, 1.0f});

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)loader->window.x / (float)loader->window.y, 0.1f, 100.0f);

    Texture tex;
    Shader shader;

public:
    explicit S1(SceneLoader *_load)
        : loader(_load), vb(5 * 36 * sizeof(float), vertices), camera({0, 0, 3}),fcam(&camera,loader->window,10,120,.11), tex(Texture(ROOT_Directory + "/res/Textures/Thing.png")),
          shader(Shader(ROOT_Directory + "/res/Shaders/InstTex.glsl"))
    {
        VertexBufferLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(2);


        va.AddBuffer(vb, layout);
        glGenBuffers(1,&offBuff);
        glBindBuffer(GL_ARRAY_BUFFER,offBuff);
        glBufferData(GL_ARRAY_BUFFER,3*3*sizeof(float),offsets,GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2,3,GL_FLOAT,false,3*sizeof(float),0);
        glVertexAttribDivisor(2,1);

        shader.Bind();
        shader.SetUniform1i("u_Texture", 0);

        loader->callbackhandler->Register(CallbackHandler::CallbackType::Render, this, OnRender);

        flags["hide_menu"] = 1;
    }

    ~S1()
    {
        loader->callbackhandler->RemoveAll(this);
    }

    static void OnRender(void *_this_, void *)
    {
        auto _this = reinterpret_cast<S1 *>(_this_);
        _this->tex.Bind();
        _this->va.Bind();

        _this->fcam.Update(_this->loader->callbackhandler->deltatime_update);
        _this->view = _this->camera.ComputeMatrix();

        _this->shader.Bind();
        _this->shader.SetUniformMat4f("u_MVP", _this->proj * _this->view * (_this->translation * _this->rotation * _this->scale));

        glDrawArraysInstanced(GL_TRIANGLES, 0, 36,3);
    }
};

extern "C" Scene *_INIT_(SceneLoader *loader)
{
    return new S1(loader);
}

extern "C" void _EXIT_(Scene *scene)
{
    delete scene;
}
