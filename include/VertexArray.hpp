#pragma once
#include <glad/glad.h>

namespace GL
{
    struct VertexBufferLayout
    {
        struct Attribute
        {
            ///For example GL_FLOAT
            GLenum type;
            ///How many values of type. For a vec3, it would be 3.
            uint count;
            ///If you store more than one value in the buffer, you need to specify where the attribute begins.
            void *offset;
        };

        ///Whether the attribute is per vertex or per instance.
        ///Currently this can only be set per buffer, as I have not found any reason to allow it to be
        ///per attribute.
        bool attribdivisor = 0;

        ///The distance between two attributes. Usually the sum of the size of all attributes.
        u_int stride;
        std::vector<Attribute> attributes;
    };

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