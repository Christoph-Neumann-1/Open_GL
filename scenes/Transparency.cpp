#include <Scene.hpp>
#include <Shader.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <glad/glad.h>
#include <Buffer.hpp>
#include <VertexArray.hpp>

using namespace GL;

class Example final : public Scene
{
    Buffer VBO;
    Buffer InstanceInfo;
    VertexArray VAO;
    Shader shader{"shader/Transparent.vs", "shader/Transparent.fs"};

    float vertices[6] = {-0.5, -0.5,
                         0.5, -0.5,
                         0, 0.5};

    struct InstanceData
    {
        glm::vec4 color;
        glm::vec3 position;
    };

    void Render()
    {
        VAO.Bind();
        shader.Bind();

        glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 4);

        VertexArray::Unbind();
        shader.UnBind();
    }

public:
    Example(SceneLoader *_loader) : Scene(_loader)
    {
        VAO.Bind();
        VBO.Bind(GL_ARRAY_BUFFER);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        VertexBufferLayout layout;
        layout.Push(GL_FLOAT, 2);
        layout.AddToVertexArray(VAO);
        shader.Bind();

        InstanceInfo.Bind(GL_ARRAY_BUFFER);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(InstanceData), nullptr, GL_STATIC_DRAW);
        VertexBufferLayout layout2;
        layout2.Push(GL_FLOAT, 4);
        layout2.Push(GL_FLOAT, 3, offsetof(InstanceData, position));
        layout2.attribdivisor = 1;
        layout2.AddToVertexArray(VAO);

        glm::vec4 color{1, 0.2, 0.8, 1};
        shader.SetUniform4f("u_Color", color);

        shader.SetUniformMat4f("u_MVP", glm::mat4(1.0f));

        RegisterFunc(CallbackType::Render, &Example::Render, this);

        VertexArray::Unbind();
        Buffer::Unbind(GL_ARRAY_BUFFER);
        shader.UnBind();
    }
};

// Calls the constructor of Example and returns a pointer to it.
SCENE_LOAD_FUNC(Example)