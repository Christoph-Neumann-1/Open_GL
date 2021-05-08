#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout(location=3) in vec3 offset;

out vec2 TexCoords;
uniform mat4 u_MVP;

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = u_MVP * vec4(aPos+offset, 1.0);
}
