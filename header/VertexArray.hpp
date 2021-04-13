///@file
#pragma once

#include "VertexBuffer.hpp"
#include "VertexBufferLayout.hpp"

/**
 * @brief Used to manage Vertexarrays and the Layout of Buffers
 */
class VertexArray
{
    unsigned int id;
    bool auto_destroy;
public:
    VertexArray(bool destroy_automatic=true);
    ~VertexArray();

    ///@return The id of the vertexarray, only use this if you need to.
    unsigned int GetId() const { return id; }
    void Bind() const;

    ///@brief Binds vertexarray 0.
    void UnBind() const;

    /**
     * @brief Add a buffer to the VertexArray.
     * 
     * Also sets the layout of the buffer according to the @ref VertexBufferLayout "layout".
     * 
     * @param buffer The Buffer can contain data or be empty
     * @param layout How the buffer is structured see VertexBufferLayout for more information.
     */
    void AddBuffer(const VertexBuffer &buffer, const VertexBufferLayout &layout);
};

