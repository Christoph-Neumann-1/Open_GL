#version 330 core
layout (location = 0) in vec3 aPos;
layout(location=3) in vec3 offset;
layout(location=4) in vec3 color;
layout(location=5) in float aRadius;

uniform mat4 u_MVP;

out vec4 v_color;

void main()
{
    gl_Position = u_MVP * vec4(aPos*aRadius+offset, 1.0);
    v_color=vec4(color,1.0);
}
