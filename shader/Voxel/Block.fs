#version 420 core

layout(location=0) out vec4 color;

uniform vec4 u_Color;
uniform sampler2DArray u_Texture;

in vec3 t_coord;


void main()
{
    vec4 tcol=texture(u_Texture,t_coord);
    color=tcol;
}