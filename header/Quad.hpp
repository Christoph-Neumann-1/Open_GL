#pragma once
#include <Drawable.hpp>
#include <glm/glm.hpp>
#include <VertexArray.hpp>
#include <VertexBuffer.hpp>
#include <VertexBufferLayout.hpp>
#include <IndexBuffer.hpp>
#include <Shader.hpp>
#include <array>
#include <stdexcept>

/**
 * @brief Draws 4 vertices
 */
class Quad : public Drawable
{
    VertexArray va;
    VertexBuffer vb;
    IndexBuffer ib;
    Shader shader;

    glm::vec2 vertices[4];
    float color[4] = {1, 1, 1, 1};

public:
    ///@param dynamic do you want to frquently write to it?
    Quad(bool dynamic = false);

    ///@brief access the vertices
    glm::vec2 &operator[](int index)
    {
        if (index < 4)
            return vertices[index];
        else
            throw std::runtime_error("Quad Index out of bounds.");
    }

    ///@brief Update the positions on the gpu.
    void Update();

    ///@brief Change color as well.
    ///@overload
    void Update(float color[4]);

    const IndexBuffer &GetIndexBuffer() const override { return ib; }

    Shader &GetShader() override { return shader; }

    const VertexArray &GetVertexArray() const override { return va; }
};