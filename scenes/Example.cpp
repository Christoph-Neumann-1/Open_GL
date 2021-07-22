/**
 * @file Example.cpp
 * @brief A short example showing the correct use of the scene system
 */

#include <Scene.hpp> //This is the only import necessary  for a scene, the rest are for drawing stuff or debug output
#include <Shader.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <glad/glad.h>
#include <Buffer.hpp>
#include <VertexArray.hpp>

//All my classes are in this namespace
using namespace GL;

//Create a new class derived from Scene
class Example final : public Scene
{
    Buffer VBO;
    VertexArray VAO;
    Shader shader{ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs"};

    //A triangle
    float vertices[6] = {-0.5, -0.5,
                         0.5, -0.5,
                         0, 0.5};

    void Render()
    {
        //Bind all buffers and the shader
        VAO.Bind();
        shader.Bind();

        //Draw 3 vertices = a triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);

        //Unbind before next draw call
        VertexArray::Unbind();
        shader.UnBind();
    }

public:
    //All the setup is done here. During loading the update thread waits so you don't have to worry about thread safety here.
    Example(SceneLoader *_loader) : Scene(_loader)
    {
        //Generate and set up Buffer
        VAO.Bind();
        VBO.Bind(GL_ARRAY_BUFFER);

        //Fill the buffer with the 3 vertices
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(float), 0); //We only have x and y coordinates, so we use 2 as a size

        shader.Bind();

        //Set the color to pink
        glm::vec4 color{1, 0.2, 0.8, 1};
        shader.SetUniform4f("u_Color", color);

        shader.SetUniformMat4f("u_MVP", glm::mat4(1.0f)); //We are already using normalized device coordinates, so an identity matrix is fine I could have used a shader without this uniform, but I didn't want yet another file.

        //Add the Render function to the callback list. This is bound automatically to make it easier to use
        RegisterFunc(CallbackType::Render, &Example::Render, this);

        //Unbind opengl stuff
        VertexArray::Unbind();
        Buffer::Unbind(GL_ARRAY_BUFFER);
        shader.UnBind();
    }

    //The threads will be synchronized here as well
    ~Example()
    {
        RemoveFunctions(); //Remove callbacks before closing scene
    }
};

// Calls the constructor of Example and returns a pointer to it.
SCENE_LOAD_FUNC(Example)