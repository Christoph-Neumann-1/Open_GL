#include <SceneLoader.hpp>
#include <Scene.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Callback.hpp>
#include <Texture.hpp>
#include <Shader.hpp>
#include <Data.hpp>

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

    float data[4][5]{0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                     0.8f, 0.0f, 0.0f, 1.0f, 0.0f,
                     0.8f, 0.6f, 0.0f, 1.0f, 1.0f,
                     0.0f, 0.6f, 0.0f, 0.0f, 1.0f};

    VertexBuffer vb;

    unsigned int indices[6] = {
        0, 1, 2,
        0, 2, 3};

    IndexBuffer ib;

    glm::vec3 c_pos = {2, 0, 2};

    glm::mat4 view = glm::lookAt(
        c_pos,
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), {0, 0, 0});
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)16 / (float)9, 0.1f, 100.0f);
    Texture tex;
    Shader shader;

public:
    explicit S1(SceneLoader *_load)
        : loader(_load), vb(5 * 36 * sizeof(float), vertices), tex(Texture(ROOT_Directory + "/res/Textures/Thing.png")), shader(Shader(ROOT_Directory + "/res/Shaders/Tex.glsl"))
    {
        VertexBufferLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(2);

        va.AddBuffer(vb, layout);

        ib.SetData(indices, 6);

        shader.Bind();
        shader.SetUniform1i("u_Texture", 0);
        shader.SetUniformMat4f("u_MVP", proj * view);

        loader->callbackhandler->Register(CallbackHandler::CallbackType::Render, this, OnRender);
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
        _this->ib.Bind();

        glm::vec3 front = glm::normalize(glm::vec3(0, 0, 0) - _this->c_pos);
        glm::vec3 right = glm::normalize(glm::cross(front, {0, 1, 0}));
        glm::vec3 up = glm::normalize(glm::cross(right, front));
        _this->c_pos += 0.1f * front * (float)glfwGetKey(_this->loader->window, GLFW_KEY_W) - 0.1f * front * (float)glfwGetKey(_this->loader->window, GLFW_KEY_S);
        _this->c_pos += 0.1f * right * (float)glfwGetKey(_this->loader->window, GLFW_KEY_D) - 0.1f * right * (float)glfwGetKey(_this->loader->window, GLFW_KEY_A);
        _this->c_pos += 0.1f * up * (float)glfwGetKey(_this->loader->window, GLFW_KEY_R) - 0.1f * up * (float)glfwGetKey(_this->loader->window, GLFW_KEY_F);

        _this->view = glm::lookAt(
            _this->c_pos,
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0));

        _this->shader.Bind();
        _this->shader.SetUniformMat4f("u_MVP", _this->proj * _this->view);

        glDrawArrays(GL_TRIANGLES, 0, 36);
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
