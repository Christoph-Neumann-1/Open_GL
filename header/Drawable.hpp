#pragma once

#include <IndexBuffer.hpp>
#include <Shader.hpp>
#include <VertexArray.hpp>

/**
 * @brief Removes duplicate functions from Renderer.
 */
class Drawable
{
public:
    virtual ~Drawable() {}

    virtual const IndexBuffer &GetIndexBuffer() const = 0;

    virtual Shader &GetShader() = 0;

    virtual const VertexArray &GetVertexArray() const = 0;
};