#include <Triangle.hpp>
#include <Data.hpp>
#include <cstring>

Triangle::Triangle(bool dynamic) : vb(6 * sizeof(float), nullptr, dynamic), shader((ROOT_Directory + "/res/Shaders/Line.glsl").c_str())
{
    unsigned int indices[] = {0, 1, 2};
    ib.SetData(indices, 3);
    va.Bind();
    vb.Bind();
    VertexBufferLayout layout;
    layout.Push<float>(2);
    va.AddBuffer(vb, layout);
    shader.Bind();
    shader.SetUniform4f("u_Color", color);
}

void Triangle::Update(float color[4])
{
    memcpy(this->color, color, 4 * sizeof(float));
    shader.Bind();
    shader.SetUniform4f("u_Color", color);

    Update();
}

void Triangle::Update()
{
    vb.Bind();
    vb.SetData(3 * 2 * sizeof(float), vertices);
}