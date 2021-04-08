#pragma once

#include <IndexBuffer.hpp>
#include <Shader.hpp>
#include <VertexArray.hpp>

/**
 * @brief Simplifies drawing of most obkects since only one class must be supported.
 * 
 * Documentation in Children
 */
class Drawable
{
public:
    virtual ~Drawable() {}

    virtual const IndexBuffer &GetIndexBuffer() const = 0;

    virtual Shader &GetShader() = 0;

    virtual const VertexArray &GetVertexArray() const = 0;
};