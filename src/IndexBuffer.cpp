///@file
#include <glad/glad.h>

#include "IndexBuffer.hpp"

IndexBuffer::IndexBuffer(const unsigned int *data, unsigned int count)
    : count(count)
{
    glGenBuffers(1, &BufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}

IndexBuffer::IndexBuffer()
    :count(0)
{
    glGenBuffers(1, &BufferID);
}

IndexBuffer::~IndexBuffer()
{
    glDeleteBuffers(1, &BufferID);
}

void IndexBuffer::Bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferID);
}

void IndexBuffer::UnBind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::SetData(const unsigned int *data, unsigned int count)
{   
    this->count=count!=0 ? count : this->count;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
}