#include "Exercise6.hlsli"

Texture2D colourTex : register(t0);
SamplerState colourSamp : register(s0);

float4 main(float3 worldPos : POSITION, float3 normal : NORMAL, float2 coord : TEXCOORD) : SV_TARGET
{
    
    normal = normalize(normal); // because it was interpolated and might not be normalized
    float3 surfaceColor = hasDiffuseTex ? colourTex.Sample(colourSamp, coord).rgb * diffuseColour.rgb : diffuseColour.rgb;
    float minNormPerLight = -dot(normal, L);
    
    float3 color;
    if (minNormPerLight > 0)
    {
        float3 observerDir = normalize(viewPos - worldPos); // V
        float3 lightReflectionDir = reflect(L, normal); // R
        
        color = Ac * surfaceColor + Kd * surfaceColor * Lc * minNormPerLight + Ks * Lc * pow (saturate(dot(observerDir,lightReflectionDir)), shininess);
        
    }else
        color = Ac * surfaceColor;
    
    return float4(color, 1.0);
    
    /*
    float3 Cd = hasDiffuseTex ? colourTex.Sample(colourSamp, coord).rgb * diffuseColour.rgb : diffuseColour.rgb;
    float3 N = normalize(normal); // because it was interpolated and might not be normalized
    float3 R = reflect(L, N);
    float3 V = normalize(viewPos - worldPos);
    float3 dotVR = saturate(dot(V, R)); // so that it is not < 0
    float dotNL = saturate(-dot(L, N)); // so that it is not < 0 (-dot(L, N) = dot(-N, L))
    
    float3 colour = Cd * Kd * dotNL * Lc + Ac * Cd + Ks * Lc * pow(dotVR, shininess);

    return float4(colour, 1.0);
    */
}