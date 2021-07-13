#version 420 core

layout(location=0) out vec4 color;

uniform vec3 u_Color;
uniform vec3 u_LightDirection;
uniform vec3 u_LightColor;
uniform float u_Ambient;
uniform vec3 u_CameraPosition;
uniform float u_Specular;
uniform float u_Shininess;

in vec3 v_normal;
in vec3 v_position;

void main()
{   
    float diff = max(dot(v_normal, -u_LightDirection), 0.0);
    vec3 diffuse = diff * u_LightColor;
    vec3 viewDir = normalize(u_CameraPosition - v_position);
    vec3 reflectDir = reflect(u_LightDirection, v_normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Shininess);
    vec3 specular = u_Specular * spec * u_LightColor; 

    vec3 result = (u_Ambient + diffuse + specular) * u_Color;

    color=vec4(result,1);
}