#version 450 core

uniform mat4 u_MVP;

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec4 vColor;
layout(location=2) in float size;

out vec4 pColor;

void main()
{
    gl_Position = u_MVP * vec4(vPos, 1.0);
    gl_PointSize = size;
    pColor = vColor;
}