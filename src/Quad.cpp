#include <Quad.hpp>
#include <Data.hpp>
#include <cstring>

Quad::Quad(bool dynamic) : vb(8 * sizeof(float), nullptr, dynamic), shader((ROOT_Directory + "/res/Shaders/Line.glsl").c_str())
{
    unsigned int indices[] = {0, 1, 2, 0, 2, 3};
    ib.SetData(indices, 6);

    va.Bind();
    vb.Bind();
    VertexBufferLayout layout;
    layout.Push<float>(2);
    va.AddBuffer(vb, layout);
    shader.Bind();
    shader.SetUniform4f("u_Color", color);
}

void Quad::Update(float color[4])
{
    memcpy(this->color, color, 4 * sizeof(float));
    shader.Bind();
    shader.SetUniform4f("u_Color", color);

    Update();
}

void Quad::Update()
{
    vb.Bind();
    vb.SetData(4 * 2 * sizeof(float), vertices);
}