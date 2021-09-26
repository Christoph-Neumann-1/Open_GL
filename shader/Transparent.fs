#version 450 core

in vec4 fragColor;

uniform sampler2D depthTexture;
uniform bool u_firstLayer;

out vec4 outColor;

void main()
{
    ivec2 texelCoord = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(depthTexture, texelCoord,0).r;
    if (gl_FragCoord.z>=depth || u_firstLayer)
    {
        outColor = fragColor;
        //Why is this always green?
        outColor=vec4(0,gl_FragCoord.z/2+0.5,0,1);
    }
    else
    {
        discard;
    }

}