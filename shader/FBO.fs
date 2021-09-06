#version 450 core

in vec2 TexCoord;
uniform sampler2D u_Texture;
uniform vec2 u_texSize;
uniform bool blur;

out vec4 FragColor;

void main()
{
    vec4 color=vec4(0);
    if(blur){
    vec2 pixelSize=1.0/u_texSize;
    vec2 offsets[25]=vec2[25](
        vec2(-2,-2)*pixelSize,
        vec2(-1,-2)*pixelSize,
        vec2(0,-2)*pixelSize,
        vec2(1,-2)*pixelSize,
        vec2(2,-2)*pixelSize,
        vec2(-2,-1)*pixelSize,
        vec2(-1,-1)*pixelSize,
        vec2(0,-1)*pixelSize,
        vec2(1,-1)*pixelSize,
        vec2(2,-1)*pixelSize,
        vec2(-2,0)*pixelSize,
        vec2(-1,0)*pixelSize,
        vec2(0,0)*pixelSize,
        vec2(1,0)*pixelSize,
        vec2(2,0)*pixelSize,
        vec2(-2,1)*pixelSize,
        vec2(-1,1)*pixelSize,
        vec2(0,1)*pixelSize,
        vec2(1,1)*pixelSize,
        vec2(2,1)*pixelSize,
        vec2(-2,2)*pixelSize,
        vec2(-1,2)*pixelSize,
        vec2(0,2)*pixelSize,
        vec2(1,2)*pixelSize,
        vec2(2,2)*pixelSize
    );

    float kernel[25]=float[25](
        0.003765,
        0.015019,
        0.023792,
        0.015019,
        0.003765,
        0.015019,
        0.059912,
        0.094907,
        0.059912,
        0.015019,
        0.023792,
        0.094907,
        0.150342,
        0.094907,
        0.023792,
        0.015019,
        0.059912,
        0.094907,
        0.059912,
        0.015019,
        0.003765,
        0.015019,
        0.023792,
        0.015019,
        0.003765
    );
    
    for(int i=0;i<25;i++)
    {
        color+=texture(u_Texture,TexCoord+offsets[i])*kernel[i];
    }
    FragColor=color;
    }
    else{
        color=texture(u_Texture,TexCoord);
    }
    FragColor=color;

}