#version 450 core

in vec4 fragColor;

uniform sampler2D depthTexture;

layout(origin_upper_left) in  vec4 gl_FragCoord;

out vec4 outColor;

void main()
{
    float depth = texture(depthTexture, gl_FragCoord.xy).r;
    if (depth < gl_FragCoord.z)
    {
        outColor = fragColor;
    }
    else
    {
        discard;
    }

}