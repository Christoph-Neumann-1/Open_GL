#pragma once

#include <VertexArray.hpp>
#include <VertexBuffer.hpp>
#include <VertexBufferLayout.hpp>
#include <IndexBuffer.hpp>
#include <Shader.hpp>
#include <glm/glm.hpp>
#include <Drawable.hpp>

/**
 * @brief A class that helps with drawing a line between 2 points.
 */
class Line : public Drawable
{
    IndexBuffer ib;
    Shader shader;
    VertexBuffer vb;
    VertexArray va;
    float *color;

public:
    glm::vec2 start;
    glm::vec2 end;
    float thickness;

    /**
     * @brief Construct a new Line from start to end
     * 
     * @param start 
     * @param end 
     * @param thickness 
     * @param color  RGBA color
     */
    Line(glm::vec2 start, glm::vec2 end, float thickness, float color[4]);

    ///@brief Updates the vertices of the curve
    void Update();

    ///@brief Sets the color as well.
    ///@overload
    void Update(float color[4]);

    ///@return a reference to the IndexBuffer.
    const IndexBuffer &GetIndexBuffer() const override
    {
        return ib;
    }

    ///@return a non-const reference to the shader.
    Shader &GetShader() override { return shader; }

    ///@returns a reference to the VertexArray.
    const VertexArray &GetVertexArray() const override { return va; }
};