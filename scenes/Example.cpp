/**
 * @file Example.cpp
 * @brief A short example showing the correct use of the scene system
 */

#include <Scene.hpp>//This is the only import necessary  for a scene, the rest are for drawing stuff or debug output
#include <Shader.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <glad/glad.h>

//All my classes are in this namespace
using namespace GL;

//Create a new class derived from Scene
class Example final : public Scene
{
    uint VBO;
    uint VAO;
    Shader shader;

    //A triangle
    float vertices[6] = {-0.5, -0.5,
                         0.5, -0.5,
                         0, 0.5};

    void Render()
    {
        //Bind all buffers and the shader
        glBindVertexArray(VAO);
        shader.Bind();

        //Draw 3 vertices = a triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);

        //Unbind before next draw call
        glBindVertexArray(0);
        shader.UnBind();
    }

public:
    //All the setup is done here. During loading the update thread waits so you don't have to worry about thread safety here.
    Example(SceneLoader *_loader) : Scene(_loader),
                                    shader(ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs")
    {
        //Generate and set up Buffer
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        //Fill the buffer with the 3 vertices
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), 0); //We only have x and y coordinates, so we use 2 as a size

        shader.Bind();

        //Set the color to pink
        float color[]{1, 0.2, 0.8, 1};
        shader.SetUniform4f("u_Color", color);

        shader.SetUniformMat4f("u_MVP", glm::mat4(1.0f)); //We are already using normalized device coordinates, so an identity matrix is fine I could have used a shader without this uniform, but I didn't want yet another file.

        //Add the Render function to the callback list. This is bound automatically to make it easier to use
        RegisterFunc(CallbackType::Render,&Example::Render, this);

        //Unbind opengl stuff
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        shader.UnBind();
    }

    //The threads will be synchronized here as well
    ~Example()
    {
        //Clean up gpu ressources
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);

        RemoveFunctions(); //Remove callbacks before closing scene
    }
};

// Calls the constructor of Example and returns a pointer to it.
SCENE_LOAD_FUNC(Example)