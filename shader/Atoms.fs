#version 420 core

layout(location=0) out vec4 color;

uniform vec3 u_Color;
uniform vec3 u_LightDirection;
uniform vec3 u_LightColor;
uniform float u_Ambient;

in vec3 v_normal;

void main()
{   
    float diff = max(dot(v_normal, -u_LightDirection), 0.0);
    vec3 diffuse = diff * u_LightColor;
    vec3 result = (u_Ambient*u_Color + diffuse) * u_Color;

    color=vec4(result,1);
}