#version 420 core

layout(location=0) in vec3 position;
layout(location=1) in vec2 texc;
uniform mat4 u_MVP;

uniform float tex_size;
out vec2 t_coord;
void main()
{
    gl_Position=u_MVP*vec4(position,1);
    t_coord=vec2((texc.x+0.5)/tex_size,(texc.y+0.5)/192);
}