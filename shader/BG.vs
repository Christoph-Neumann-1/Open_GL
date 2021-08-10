//Copyright © 2021 Christoph Neumann - MIT License
#version 450 core
layout (location = 0) in vec2 aPos;

out vec2 TexCoords;
uniform mat4 u_MVP;
uniform float u_zOffset;

void main()
{
    TexCoords =(u_MVP * vec4(aPos,0,1)).xy; 
    gl_Position = vec4(aPos, u_zOffset, 1.0);
}
