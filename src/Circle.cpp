#include <Circle.hpp>
#include <Data.hpp>
#include <cmath>

Circle::Circle(float radius, unsigned int segcount, const glm::vec2 &origin, const std::string &shader_path)
    : ib(nullptr, 3 * segcount), shader(ROOT_Directory + shader_path), vb((segcount + 1) * 2 * sizeof(float),nullptr)
{
    Update(radius, segcount, origin);

    va.Bind();
    VertexBufferLayout layout;
    layout.Push<float>(2);
    va.AddBuffer(vb, layout);
}

void Circle::Update(float radius, unsigned int segcount, const glm::vec2 &origin)
{
    ib.Bind();
    unsigned int *indices = static_cast<unsigned int *>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));
    unsigned int offset = 0;

    for (u_int i = 1; i < segcount; i++)
    {
        indices[offset++] = 0;
        indices[offset++] = i;
        indices[offset++] = i + 1;
    }

    indices[offset++] = 0;
    indices[offset++] = segcount;
    indices[offset] = 1;

    vb.Bind();
    float *vertices = static_cast<float *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

    vertices[0] = origin.x;
    vertices[1] = origin.y;
    offset = 2;

    for (u_int i = 1; i <= segcount; i++)
    {
        vertices[offset++] = origin.x + radius * cos(i * 2 * M_PI / segcount);
        vertices[offset++] = origin.y + radius * sin(i * 2 * M_PI / segcount);
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}