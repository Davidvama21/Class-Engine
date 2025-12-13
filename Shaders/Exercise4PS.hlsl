Texture2D colourTex : register(t0); // our texture
SamplerState colourSampler : register(s0);

float4 main(float2 texCoords : TEXCOORD) : SV_TARGET
{
    return colourTex.Sample(colourSampler, texCoords); // sets the pixel colour from the texture sampled
}