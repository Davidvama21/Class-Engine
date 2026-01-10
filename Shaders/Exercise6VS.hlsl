#include "Exercise6.hlsli"

cbuffer MVP : register(b0)
{
    float4x4 mvp; // mvp = model * view * projection
};

struct VertexOutput
{
    float3 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;    // required so that the
    float4 position : SV_POSITION; // GPU knows how to threat the components (common variable between shaders, and hard-coded ones from the pipeline)
    
};

VertexOutput main(float3 position : POSITION, float2 texCoord : TEXCOORD, float3 normal : NORMAL)
{
    VertexOutput output;
    
    output.texCoord = texCoord; // pass vertex texture coordinate
    output.position = mul(float4(position, 1.0), mvp); // Pre-multiply vector by MVP matrix
    
    output.normal = mul(normal, (float3x3) normalMat); // pass normal to world coordinates
    output.worldPos = mul(float4(position, 1.0), modelMat); // pass model pos to world coordinates
    
    return output;
}