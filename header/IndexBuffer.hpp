///@file
#pragma once

/**
 * @brief Abstraction to create and manipulate Indexbuffers
 */
class IndexBuffer
{
private:
    unsigned int BufferID;
    unsigned int count;
    bool destroy;

public:
    /**
 * @brief Generate a new Buffer and fill it with data.
 * 
 * @param data what to fill it with
 * @param count how many indices are supposed to be in the buffer
 */
    IndexBuffer(const unsigned int *data, unsigned int count, bool destroy_automatic = true);

    ///@brief Construct a new Index Buffer object without data
    IndexBuffer(bool destroy_automatic = true);

    ~IndexBuffer();
    ///@brief Binds the buffer.
    void Bind() const;

    ///@brief Binds Buffer 0 (Unbinds it)
    void UnBind() const;

    /**
     * @brief Set the index data.
     * 
     * @param data what to fill it with
     * @param count how many indices are supposed to be in the buffer
     */
    void SetData(const unsigned int *data, unsigned int count);

    ///@return The number of indices in the buffer.
    inline unsigned int GetCount() const { return count; }

    unsigned int GetId() { return BufferID; }
};