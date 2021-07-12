#version 420 core

layout(location=0) in vec3 position;
layout(location=3) in vec3 offset;

uniform float u_scale;
uniform mat4 u_MVP;

void main()
{
    gl_Position=u_MVP*vec4(position*u_scale+offset,1);
}