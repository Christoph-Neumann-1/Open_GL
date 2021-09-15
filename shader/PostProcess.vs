#version 450 core

layout (location = 0) in vec3 position;

out vec2 texCoord;

uniform float u_zOffset=0.0;

void main()
{
    gl_Position = vec4(position+vec3(0,0,u_zOffset), 1.0);
    texCoord = position.xy * 0.5 + 0.5;
}