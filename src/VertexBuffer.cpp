///@file

#include "VertexBuffer.hpp"

VertexBuffer::VertexBuffer(unsigned int size, const void *data, bool dynamic)
{
    glGenBuffers(1, &BufferID);
    glBindBuffer(GL_ARRAY_BUFFER, BufferID);
    glBufferData(GL_ARRAY_BUFFER, size, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}