//Copyright © 2021 Christoph Neumann - MIT License
#version 420 core

layout(location=0) out vec4 color;

uniform vec4 u_Color;

void main()
{
    color=u_Color;
}