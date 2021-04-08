#pragma once

#include <Drawable.hpp>
#include <glm/glm.hpp>

/**
 * @brief Draws a cirle by using polygons with lots of sides
 */
class Circle : public Drawable
{
    IndexBuffer ib;
    Shader shader;
    VertexBuffer vb;
    VertexArray va;

public:
    /**
     * @brief Construct a circle
     * 
     * @param radius radius in whatever coordinatesystem the renderer uses
     * @param segcount how many lines to draw
     * @param origin where to draw the circle I recommend using matrices for transfor insead of this.
     */
    explicit Circle(float radius, unsigned int segcount = 64, const glm::vec2 &origin = {0, 0});

    ///@brief Set the color RGBA
    void SetColor(const float color[4])
    {
        shader.Bind();
        shader.SetUniform4f("u_Color", color);
    }

    ///@brief same as constructor
    void Update(float radius, unsigned int segcount, const glm::vec2 &origin = {0, 0});

    /// @return a const reference to the IndexBuffer
    const IndexBuffer &GetIndexBuffer() const override { return ib; }

    ///@return a reference to this curves Shader. The shader can be modified.
    Shader &GetShader() override { return shader; }

    ///@return a const reference to the VertexArray
    const VertexArray &GetVertexArray() const override { return va; }
};