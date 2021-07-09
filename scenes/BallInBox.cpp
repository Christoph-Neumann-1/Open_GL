#include <Scene.hpp>
#include <ModelLoader.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Data.hpp>
#include <Camera/Camera3D.hpp>
#include <Camera/Fplocked.hpp>

using namespace GL;

class BallInBox : public Scene
{
    Model ball{ROOT_Directory + "/res/Models/sphere.obj"};
    Shader shader{ROOT_Directory + "/shader/Default.vs", ROOT_Directory + "/shader/Default.fs"};

    uint vb, va;

    Camera3D camera{{0, 0, 1}};
    Fplocked fplocked{&camera, loader->GetWindow()};

    glm::mat4 wall_transforms[1]{
        glm::translate(glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(1, 0, 0)), {0, 0, -1})};

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)loader->GetWindow().GetWidth() / (float)loader->GetWindow().GetHeigth(), 0.1f, 100.0f);

    struct Side
    {
        std::array<glm::vec3, 6> vertices;
        Side(std::array<glm::vec3, 6> v) : vertices(v) {}
    };

    Side sides[5]{
        {std::array<glm::vec3, 6>{glm::vec3(1, -1, -1), glm::vec3(1, 1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, 1, -1), glm::vec3(-1, -1, -1), glm::vec3(1, -1, -1)}}, //back
        {std::array<glm::vec3, 6>{glm::vec3(1, 1, 1), glm::vec3(-1, 1, 1), glm::vec3(-1, 1, -1), glm::vec3(1, 1, 1), glm::vec3(-1, 1, -1), glm::vec3(1, 1, -1)}},       //top
        {std::array<glm::vec3, 6>{glm::vec3(-1, -1, 1), glm::vec3(1, -1, 1), glm::vec3(1, -1, -1), glm::vec3(-1, -1, 1), glm::vec3(1, -1, -1), glm::vec3(-1, -1, -1)}}, //bottom
        {std::array<glm::vec3, 6>{glm::vec3(-1, 1, 1), glm::vec3(-1, -1, 1), glm::vec3(-1, -1, -1), glm::vec3(-1, 1, 1), glm::vec3(-1, -1, -1), glm::vec3(-1, 1, -1)}}, //left
        {std::array<glm::vec3, 6>{glm::vec3(1, 1, 1), glm::vec3(1, 1, -1), glm::vec3(1, -1, -1), glm::vec3(1, -1, -1), glm::vec3(1, -1, 1), glm::vec3(1, 1, 1)}}        //right
    };

    glm::vec4 side_colors[5]{
        glm::vec4(0.0f, 0.8f, 0.0f, 1), //back
        glm::vec4(0.2f, 0.0f, 1.0f, 1), //top
        glm::vec4(0.2f, 0.0f, 1.0f, 1), //bottom
        glm::vec4(1.0f, 0.2f, 0.0f, 1), //left
        glm::vec4(1.0f, 0.2f, 0.0f, 1)  //right
    };

    void RenderBox()
    {
        glBindVertexArray(va);

        fplocked.Update(loader->GetTimeInfo().RenderDeltaTime());

        shader.SetUniformMat4f("u_MVP", proj * camera.ComputeMatrix());

        for (int i = 0; i < 5; i++)
        {
            shader.SetUniform4f("u_Color", (float*)&side_colors[i]);
            glDrawArrays(GL_TRIANGLES, 6*i, 6);
        }


        glBindVertexArray(0);
    }

    void Render()
    {
        shader.Bind();
        RenderBox();
        shader.UnBind();
    }

public:
    BallInBox(SceneLoader *_loader) : Scene(_loader)
    {
        glGenVertexArrays(1, &va);
        glBindVertexArray(va);
        glGenBuffers(1, &vb);
        glBindBuffer(GL_ARRAY_BUFFER, vb);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

        glBufferData(GL_ARRAY_BUFFER, sizeof(sides), sides, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        RegisterFunc(CallbackType::Render, &BallInBox::Render, this);
    }
    ~BallInBox()
    {
        glDeleteBuffers(1, &vb);
        glDeleteVertexArrays(1, &va);
        RemoveFunctions();
    }
};

SCENE_LOAD_FUNC(BallInBox)