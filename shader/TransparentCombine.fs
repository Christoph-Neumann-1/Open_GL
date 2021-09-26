#version 450 core

layout (location = 0) out vec4 color;

uniform sampler2D u_texture;
uniform vec2 u_screenSize;

void main()
{
    color = texture(u_texture, vec2(gl_FragCoord.xy) / u_screenSize);
    // color=texelFetch(u_texture, ivec2(gl_FragCoord.xy), 0);
}