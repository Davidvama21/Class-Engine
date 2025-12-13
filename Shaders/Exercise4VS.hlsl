cbuffer Transforms : register(b0)
{
    float4x4 mvp; // mvp = model * view * projection
};

struct VertexOutput
{
    float2 texCoord : TEXCOORD;    // required so that the
    float4 position : SV_POSITION; // GPU knows how to threat the components (common variable between shaders, and hard-coded ones from the pipeline)
};

VertexOutput main(float3 position : POSITION, float2 texCoord : TEXCOORD )
{
    VertexOutput output;
    
    output.texCoord = texCoord; // pass vertex texture coordinate
    output.position = mul(float4(position, 1.0), mvp); // Pre-multiply vector by MVP matrix
    
    return output;
}