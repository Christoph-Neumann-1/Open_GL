///@file
#include <Line.hpp>
#include <cmath>
#include <Data.hpp>
#include <cstring>

Line::Line(glm::vec2 start, glm::vec2 end, float thickness, float color[4])
    : shader(Shader(ROOT_Directory + "/res/Shaders/Line.glsl")),vb(8*sizeof(float)), start(start), end(end), thickness(thickness)
{
    {
        unsigned int idata[] = {0, 1, 2, 1, 2, 3};
        ib.SetData(idata, 6);
    }
    va.Bind();
    vb.Bind();
    VertexBufferLayout layout;
    layout.Push<float>(2);
    va.AddBuffer(vb, layout);

    Update(color);
}

void Line::Update(float color[4])
{
    shader.Bind();
    shader.SetUniform4f("u_Color", color);
    memcpy(this->color, color, 4 * sizeof(float));
    Update();
}

void Line::Update()
{
    glm::vec2 nv = end - start;
    float length = glm::length(nv);
    nv = glm::vec2(-nv.y / length, nv.x / length);
    nv *= thickness;
    glm::vec2 vertices[] = {start + nv, start - nv, end + nv, end - nv};
    vb.Bind();
    vb.SetData(8 * sizeof(float), &vertices[0].x);
    shader.Bind();
    shader.SetUniform4f("u_Color", color);
}