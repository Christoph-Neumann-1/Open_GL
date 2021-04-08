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
 * @brief Generate and draw a triangle. 
 */
class Triangle : public Drawable
{
    VertexArray va;
    VertexBuffer vb;
    IndexBuffer ib;
    Shader shader;

    glm::vec2 vertices[3];
    float color[4] = {1, 1, 1, 1};

public:
    ///@param dynamic true if you want to write to it frequently.
    Triangle(bool dynamic = false);

    ///@brief Easy access to the vertices.
    glm::vec2 &operator[](int index)
    {
        if (index < 3)
            return vertices[index];
        else
            throw std::runtime_error("Triangle Index out of bounds.");
    }

    ///@brief Update the data on the GPU.
    void Update();

    ///@brief Change color as well. @overload
    void Update(float color[4]);

    const IndexBuffer &GetIndexBuffer() const override { return ib; }

    Shader &GetShader() override { return shader; }

    const VertexArray &GetVertexArray() const override { return va; }
};