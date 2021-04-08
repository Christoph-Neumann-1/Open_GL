///@file
#pragma once

#include <vector>
#include <glad/glad.h>

/**
 * @brief Specifies the type of data in the buffer. Used in VertexBufferLayout.
 */
struct VertexBufferElement
{
    unsigned int type;  ///<GLEnum supported values are GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_INT
    unsigned int count; ///<How large should this element be
    bool normalized;    ///<Should the data be normalized

    ///@return Size in bytes of supported GLEnums.
    static unsigned int GetSizeOfType(unsigned int type)
    {
        switch (type)
        {
        case GL_FLOAT:
            return 4;
        case GL_UNSIGNED_BYTE:
            return 1;
        case GL_UNSIGNED_INT:
            return 4;
        default:
            return 0;
        }
    }
};

///@brief specifies the layout of an VertexBuffer using VertexBufferElement can be passed to VertexArray.
class VertexBufferLayout
{
    std::vector<VertexBufferElement> elements;
    unsigned int stride;

public:
    ///@brief This has no purpose.
    VertexBufferLayout()
        : stride(0)
    {
    }

    ///@brief Add count elemts of T to the layout. Please look at overrides.
    template <typename T>
    void Push(unsigned int count);

    ///@return a Vector containing all elements.
    inline const std::vector<VertexBufferElement> &GetElements() const { return elements; }

    ///@return The total stride of the buffer.
    inline unsigned int GetStride() const { return stride; }
};

#pragma region

///@overload
template <>
inline void VertexBufferLayout::Push<float>(unsigned int count)
{
    elements.emplace_back(GL_FLOAT, count, false);
    stride += VertexBufferElement::GetSizeOfType(GL_FLOAT) * count;
}

///@overload
template <>
inline void VertexBufferLayout::Push<unsigned int>(unsigned int count)
{
    elements.emplace_back(GL_UNSIGNED_INT, count, false);
    stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT) * count;
}

///@overload
template <>
inline void VertexBufferLayout::Push<unsigned char>(unsigned int count)
{
    elements.emplace_back(GL_UNSIGNED_BYTE, count, false);
    stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_BYTE) * count;
}

#pragma endregion