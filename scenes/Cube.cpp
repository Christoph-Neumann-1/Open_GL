#include <SceneLoader.hpp>
#include <Scene.hpp>
#include <Renderer.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Callback.hpp>
#include <Texture.hpp>
#include <Shader.hpp>
#include <Data.hpp>
#include <glm/gtx/quaternion.hpp>

float c_speed = 0.06f;
float c_rspeed = 1.0f;
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

    VertexBuffer vb;

    IndexBuffer ib;

    glm::vec3 c_pos = {0, 0, 3};
    glm::vec3 c_dir = {0, 0, -1};

    glm::quat c_rot{1, 0, 0, 0};

    glm::mat4 view = glm::lookAt(
        c_pos,
        c_pos + c_dir,
        glm::vec3(0, 1, 0));

    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3{0, 0, 0});
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), {1.0f, 1.0f, 1.0f});

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)loader->window.x / (float)loader->window.y, 0.1f, 100.0f);

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

        shader.Bind();
        shader.SetUniform1i("u_Texture", 0);
        shader.SetUniformMat4f("u_MVP", proj * view * (translation * rotation * scale));

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

        glm::vec3 right = glm::normalize(glm::cross(_this->c_dir, {0, 1, 0}));
        glm::vec3 up = {0, 1, 0};
        glm::vec3 forward=_this->c_pos + glm::vec3(glm::vec4{0, 0, -1, 0} * glm::toMat4(_this->c_rot));

        _this->c_pos += c_speed * _this->c_dir * (float)glfwGetKey(_this->loader->window, GLFW_KEY_W) - c_speed * _this->c_dir * (float)glfwGetKey(_this->loader->window, GLFW_KEY_S);
        _this->c_pos += c_speed * right * (float)glfwGetKey(_this->loader->window, GLFW_KEY_D) - c_speed * right * (float)glfwGetKey(_this->loader->window, GLFW_KEY_A);
        _this->c_pos += c_speed * up * (float)glfwGetKey(_this->loader->window, GLFW_KEY_R) - c_speed * up * (float)glfwGetKey(_this->loader->window, GLFW_KEY_F);

        _this->c_rot = glm::rotate(_this->c_rot, glm::radians(c_rspeed * (float)(glfwGetKey(_this->loader->window, GLFW_KEY_E) - glfwGetKey(_this->loader->window, GLFW_KEY_Q))), up);
        _this->c_rot = glm::rotate(_this->c_rot, glm::radians(c_rspeed * (float)(glfwGetKey(_this->loader->window, GLFW_KEY_X) - glfwGetKey(_this->loader->window, GLFW_KEY_C))), right);
        _this->c_rot =glm::rotate(_this->c_rot, glm::radians(c_rspeed * (float)(glfwGetKey(_this->loader->window, GLFW_KEY_T) - glfwGetKey(_this->loader->window, GLFW_KEY_G))), forward);

    
        _this->view = glm::lookAt(
            _this->c_pos,
            forward,
            glm::vec3(glm::vec4{0, 1, 0,0}*glm::toMat4(_this->c_rot)));

        _this->shader.Bind();
        _this->shader.SetUniformMat4f("u_MVP", _this->proj * _this->view * (_this->translation * _this->rotation * _this->scale));

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
