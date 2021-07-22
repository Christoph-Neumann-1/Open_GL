#include <glad/glad.h>

namespace GL
{
    /**
 * @brief This wrapper can't do much, but it is better than deleting buffers myself in the destructor.
 * 
 * The current version creates the buffer and deletes it.
 *  It also offers a bind and unbind function as well as access to the id of the buffer.
 */
    class Buffer
    {
        uint bufferId{0};

    public:
        Buffer() { glGenBuffers(1, &bufferId); }
        ~Buffer() { glDeleteBuffers(1, &bufferId); }
        void Bind(GLenum target) { glBindBuffer(target, bufferId); }
        static void Unbind(GLenum target) { glBindBuffer(target, 0); }
        const uint GetId() { return bufferId; }
        //Same as GetId, but it makes working with GL functions a bit easier.
        operator uint () { return bufferId; }
    };
}