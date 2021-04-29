#include <Scene.hpp>
#include <glad/glad.h>

using namespace GL;
class t_Scene final : public Scene
{
    uint VBO;
    uint VAO;
    uint Program;

    void Render()
    {
        glBindVertexArray(VAO);
        glUseProgram(Program);
        glDrawArrays(GL_TRIANGLES,0,3);
    }

public:
    t_Scene(SceneLoader *_loader) : Scene(_loader)
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        float vertices[]{0, 0, 1, 0, 1, 1};
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);

        auto vertex = glCreateShader(GL_VERTEX_SHADER);
        auto vsrc = "#version 450\nlayout(location=0) in vec4 pos;void main(){gl_Position=pos;}";
        glShaderSource(vertex, 1, &vsrc, NULL);
        glCompileShader(vertex);
        auto fragment = glCreateShader(GL_FRAGMENT_SHADER);
        auto fsrc = "#version 450\nlayout(location=0) out vec4 color;void main(){color=vec4(1,1,0,1);}";
        glShaderSource(fragment, 1, &fsrc, NULL);
        glCompileShader(fragment);
        Program = glCreateProgram();
        glAttachShader(Program, vertex);
        glAttachShader(Program, fragment);
        glLinkProgram(Program);

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        loader->GetCallback().GetList(CallbackType::Render).Add(std::bind(&t_Scene::Render,this),callback_id);
    }
    ~t_Scene()
    {
        glDeleteProgram(Program);
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void PrepareUnload() final override
    {
        loader->GetCallback().RemoveAll(callback_id);
    }

    void Terminate() final override
    {
        loader->GetCallback().RemoveAll(callback_id);
    }
};

extern "C"
{
    Scene *_LOAD_(SceneLoader *loader)
    {
        return new t_Scene(loader);
    }
}