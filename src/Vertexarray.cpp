///@file
#include <glad/glad.h>

#include "VertexArray.hpp"


VertexArray::VertexArray()
{
    glGenVertexArrays(1, &id);
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &id);
}

void VertexArray::Bind() const
{
    glBindVertexArray(id);
}

void VertexArray::UnBind() const
{
    glBindVertexArray(0);
}

void VertexArray::AddBuffer(const VertexBuffer &buffer, const VertexBufferLayout &layout)
{   
    Bind();
    buffer.Bind();
    const auto &elements = layout.GetElements();
    unsigned long offset =0;
    for (unsigned int i = 0; i < elements.size(); i++)
    {   
        const auto& element =elements[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i,element.count,element.type,element.normalized, layout.GetStride(),(const void*)offset);
        offset+=element.count*VertexBufferElement::GetSizeOfType(element.type);
    }
}