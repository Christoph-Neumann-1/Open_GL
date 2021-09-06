#version 450 core

in vec2 TexCoord;
uniform sampler2D u_Texture;

out vec4 FragColor;

void main()
{
    FragColor = texture(u_Texture, TexCoord);
}