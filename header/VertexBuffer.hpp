///@file
#pragma once
#include <glad/glad.h>

/**
 * @brief Use this to manipulate a vertexbuffer.
 * 
 * Only abstracts basic things like binding, and setting data.
 */
class VertexBuffer
{
private:
    unsigned int BufferID;

public:
    /**
     * @brief Construct a new VertexBuffer and initialize it with data.
     * 
     * @param size How many bytes to allocate
     * @param data a pointer to the data
     */
    explicit VertexBuffer(unsigned int size, const void *data = nullptr, bool dynamic = false);

    ///@brief Construct a VertexBuffer, but leave it empty.
    VertexBuffer() { glGenBuffers(1, &BufferID); }
    ~VertexBuffer() { glDeleteBuffers(1, &BufferID); }

    /**
     * @brief Fill the buffer with data.
     * 
     * @param size How many bytes to write
     * @param data a pointer to the data
     * @param offset where to start writing
     */
    void SetData(unsigned int size, const void *data, unsigned int offset = 0) { glBufferSubData(GL_ARRAY_BUFFER, offset, size, data); }

    /**
     * @brief Fill the buffer with data and resize it.
     * 
     * Use this for static Updates.
     * 
     * @param size How many bytes to allocate
     * @param data a pointer to the data
     * @param dynamic if you want to allocate a dynamic buffer
     */
    void SetSize(unsigned int size, const void *data = nullptr, bool dynamic = false) { glBufferData(GL_ARRAY_BUFFER, size, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW); }

    ///@brief Bind the Buffer necessary befor all calls.
    void Bind() const { glBindBuffer(GL_ARRAY_BUFFER, BufferID); }

    ///@brief Bind buffer 0.
    void UnBind() const { glBindBuffer(GL_ARRAY_BUFFER, 0); }

    unsigned int GetId() { return BufferID; }
};