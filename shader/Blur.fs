#version 450 core

layout (location =0) out vec4 color;

in vec2 texCoord;

uniform sampler2D u_texture;
uniform bool u_vertical;
uniform float u_offset;
uniform float u_kernel[25];

void main()
{
    vec4 sum = vec4(0.0);
    if(u_vertical)
    {
        for(int i = -12; i <= 12; i++)
        {
            sum += texture(u_texture, texCoord + vec2(0.0, u_offset * float(i)) ) * u_kernel[i + 12];
        }
    }
    else
    {
        for(int i = -12; i <= 12; i++)
        {
            sum += texture(u_texture, texCoord + vec2(u_offset * float(i), 0.0) ) * u_kernel[i + 12];
        }
    }
    color = sum;
}