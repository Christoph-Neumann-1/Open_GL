#pragma once
#include <glad/glad.h>

namespace GL
{

    //TODO: add functions to add buffers and set vertex attribs
    /**
    * @brief Handles creating and destroying vertex array objects.
    * 
    * The current version creates the vertexarray and deletes it.
    *  It also offers a bind and unbind function as well as access to the id of the vertexarray.
    */
    class VertexArray
    {
        uint id{0};

    public:
        VertexArray() { glGenVertexArrays(1, &id); }
        ~VertexArray() { glDeleteVertexArrays(1, &id); }
        void Bind() { glBindVertexArray(id); }
        static void Unbind() { glBindVertexArray(0); }
        const uint GetId() { return id; }
        //Same as GetId, but it makes working with GL functions a bit easier.
        operator uint() { return id; }
    };
}