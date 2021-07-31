#pragma once
#include <glad/glad.h>
#include <Buffer.hpp>

namespace GL
{
    class VertexArray;
    //TODO: add a function that adds this buffer to a vertex array
    //TODO: auto stride and offset use template to switch between manual and automatic
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

            Attribute(GLenum type_, uint count_, void *offset_) : type(type_), count(count_), offset(offset_) {}
        };

        ///Whether the attribute is per vertex or per instance.
        ///Currently this can only be set per buffer, as I have not found any reason to allow it to be
        ///per attribute.
        uint attribdivisor = 0;

        ///The distance between two attributes. Usually the sum of the size of all attributes.
        u_int stride{0};
        std::vector<Attribute> attributes;

        void Push(Attribute attrib)
        {
            attributes.push_back(attrib);
        }

        void AddToVertexArray(VertexArray &array);

        void BindAndAddToVertexArray(VertexArray &array, Buffer &buffer);
    };

    //TODO: add functions to add buffers and set vertex attribs
    //TODO: keep track of the current vertex attribs
    /**
    * @brief Handles creating and destroying vertex array objects.
    * 
    * The current version creates the vertexarray and deletes it.
    *  It also offers a bind and unbind function as well as access to the id of the vertexarray.
    */
    class VertexArray
    {
        uint id{0};
        uint currentAttribIndex{0};

    public:
        VertexArray() { glGenVertexArrays(1, &id); }
        ~VertexArray() { glDeleteVertexArrays(1, &id); }
        void Bind() { glBindVertexArray(id); }
        static void Unbind() { glBindVertexArray(0); }
        const uint GetId() { return id; }
        //Same as GetId, but it makes working with GL functions a bit easier.
        operator uint() { return id; }
        //This function assumes that the buffer and the vertex array are already bound.
        void AddAttrib(const VertexBufferLayout::Attribute &attrib, const VertexBufferLayout &layout)
        {
            glEnableVertexAttribArray(currentAttribIndex);
            glVertexAttribPointer(currentAttribIndex, attrib.count, attrib.type, false, layout.stride, attrib.offset);
            glVertexAttribDivisor(currentAttribIndex, layout.attribdivisor);
            currentAttribIndex++;
        }
        void SetCurrentAttrib(const uint index) { currentAttribIndex = index; }
        uint GetCurrentAttrib() { return currentAttribIndex; }
    };

    inline void VertexBufferLayout::BindAndAddToVertexArray(VertexArray &array, Buffer &buffer)
    {
        buffer.Bind(GL_ARRAY_BUFFER);
        array.Bind();
        AddToVertexArray(array);
        buffer.Unbind(GL_ARRAY_BUFFER);
        array.Unbind();
    }

    inline void VertexBufferLayout::AddToVertexArray(VertexArray &array)
    {
        for (auto &attribute : attributes)
        {
            array.AddAttrib(attribute, *this);
        }
    }
}