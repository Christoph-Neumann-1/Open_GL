#version 450 core

in vec4 pColor;
layout (location = 0) out vec4 color;

void main()
{
    color = pColor;
}