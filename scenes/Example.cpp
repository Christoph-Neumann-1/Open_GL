#include <Scene.hpp>
#include <Shader.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <glad/glad.h>

using namespace GL;

//Create a new Scene class.
class Example final : public Scene
{
    uint VBO;
    uint VAO;
    Shader shader;

    float vertices[6] = {-0.5, -0.5,
                         0.5, -0.5,
                         0, 0.5};

    void Render()
    {
        //Bind all ressources
        glBindVertexArray(VAO);
        shader.Bind();

        //Draw 3 vertices = a triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);

        //Unbind before next draw call
        glBindVertexArray(0);
        shader.UnBind();
    }

public:
    Example(SceneLoader *_loader) : Scene(_loader),
                                    shader(ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs")
    {
        //Generate and set up Buffer
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), 0); //We only have x and y coordinates, so we use 2 as a size

        shader.Bind();

        //Set the color to pink
        float color[]{1, 0.2, 0.8, 1};
        shader.SetUniform4f("u_Color", color);

        shader.SetUniformMat4f("u_MVP", glm::mat4(1.0f)); //We are already using normalized device coordinates, so an identity matrix is fine

        //Add the Render function to the callback list.
        RegisterFunc(std::bind(&Example::Render, this), CallbackType::Render);

        //Unbind opengl stuff
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        shader.UnBind();
    }

    ~Example()
    {
        //Clean up gpu ressources
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);

        loader->GetCallback().RemoveAll(callback_id); //Remove callbacks before closing scene
    }
};

// Calls the constructor of Example
SCENE_LOAD_FUNC(Example)