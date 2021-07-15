#version 420 core

layout(location=0) in vec4 position;
layout(location=1) in vec2 offset;
layout(location=2) in vec4 color;
uniform mat4 u_MVP;
uniform mat4 scale_mat;

out vec4 Color;

void main()
{
    gl_Position=u_MVP*(scale_mat*position+vec4(offset,0,0));
    Color=color;
}