//Copyright © 2021 Christoph Neumann - MIT License
#version 420 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=3) in vec3 offset;

uniform mat4 u_P;
uniform mat4 u_M;
uniform mat4 u_V;

out vec3 v_normal;
out vec3 v_position;

void main()
{
    gl_Position=u_P*u_V*(u_M*vec4(position,1)+vec4(offset,0));
    v_normal=normalize(normal);
    v_position=(u_M*vec4(position,0)+vec4(offset,0)).xyz;
}