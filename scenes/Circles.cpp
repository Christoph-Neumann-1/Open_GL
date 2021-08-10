/**
 * @file Circles.cpp
 * @brief I checked if the Circle class is working.
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#include <Scene.hpp>
#include <Data.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Primitives/Circle.hpp>

using namespace GL;

class CircleScene final : public Scene
{
    Circle circle{ROOT_Directory+"/shader/Default.vs", ROOT_Directory+"/shader/Default.fs"};
    void Render()
    {
        circle.shader.Bind();
        circle.VAO.Bind();
        glDrawArrays(GL_TRIANGLE_FAN, 0, circle.vcount);
        VertexArray::Unbind();
        circle.shader.UnBind();
    }

public:
    CircleScene(SceneLoader *_loader) : Scene(_loader)
    {
        circle.ComputeVertices(0.5f, 100);
        circle.shader.Bind();
        circle.shader.SetUniform4f("u_Color", {1, 0.5, 0.8, 1});
        circle.shader.SetUniformMat4f("u_MVP", glm::mat4(1));
        circle.shader.UnBind();

        RegisterFunc(CallbackType::Render, &CircleScene::Render, this);
    }
    ~CircleScene()
    {
        RemoveFunctions();
    }
};

SCENE_LOAD_FUNC(CircleScene)