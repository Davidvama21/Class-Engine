#include "Exercise6.hlsli"

Texture2D colourTex : register(t0);
SamplerState colourSamp : register(s0);

float4 main(float3 worldPos : POSITION, float3 normal : NORMAL, float2 coord : TEXCOORD) : SV_TARGET
{
    normal = normalize(normal); // because it was interpolated and might not be normalized
    float3 surfaceColor = hasDiffuseTex ? colourTex.Sample(colourSamp, coord).rgb * diffuseColour.rgb : diffuseColour.rgb;
    float minNormPerLight = dot(-normal, L);
    
    float3 color;
    if (minNormPerLight > 0)
    {
        float3 observerDir = normalize(viewPos - worldPos); // V
        float3 lightReflectionDir = reflect(L, normal); // R
        
        color = Ac * surfaceColor + Kd * surfaceColor * Lc * minNormPerLight + Ks * Lc * pow (dot(observerDir,lightReflectionDir), shininess);
        
    }else
        color = Ac * surfaceColor;
    
    return float4(color, 1.0);
}