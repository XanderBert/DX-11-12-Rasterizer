// Output structure for the vertex shader
struct VertexOutput
{
    float4 Color : COLOR;
    float4 position : SV_POSITION;
};

// Constant buffer structure
// cbuffer ConstantBuffer : register(b0)
// {
//     // Define your constant buffer variables here
// };

// Vertex shader function
VertexOutput main(float3 pos : POSITION, float4 color : COLOR)
{
    VertexOutput output;

    output.Color = color;
    output.position = float4(pos, 1.0f);

    return output;
}