#version 450 core
layout (location =0) out vec4 color;

in vec2 texCoord;
uniform sampler2D u_texture;

void main()
{
    color = texture(u_texture, texCoord);
}