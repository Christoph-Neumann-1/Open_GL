#version 450 core

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec3 offset;

out vec4 fragColor;

void main()
{
    fragColor = color;
    gl_Position = vec4(vertexPos + offset, 1.0);
}