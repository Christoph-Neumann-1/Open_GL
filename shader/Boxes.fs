#version 420 core

layout(location=0) out vec4 color;

in vec4 Color;

void main()
{
    color=Color;
}