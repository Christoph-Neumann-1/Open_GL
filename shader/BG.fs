#version 450 core 

layout(location=0) out vec4 color;

uniform sampler2D u_texture;

in vec2 TexCoords;

void main() {
    color = texture(u_texture, TexCoords);
}