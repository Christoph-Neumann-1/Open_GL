//Copyright © 2021 Christoph Neumann - MIT License
#version 420 core

layout(location=0) out vec4 color;

uniform sampler2DArray u_Texture;
uniform vec3 u_Color;

in float is_drawn;
in vec3 t_coord;


void main()
{
    vec4 tcol=texture(u_Texture,t_coord);
    color=vec4(tcol.rgb+u_Color,tcol.a*is_drawn);
}