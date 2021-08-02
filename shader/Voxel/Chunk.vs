#version 420 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 texc;
uniform mat4 u_MVP;

uniform vec2 u_player_pos;
uniform float u_view_dist;

out vec3 t_coord;
out float is_drawn;

void main()
{
    gl_Position=u_MVP*vec4(position,1);
    t_coord=texc;
    float dist=length(u_player_pos-position.xz);
    is_drawn=float(dist<u_view_dist);
}