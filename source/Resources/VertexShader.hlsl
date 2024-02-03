// Output structure for the vertex shader

struct VertexOutput
{
    float4 Color : COLOR;
    float4 position : SV_POSITION;
};

// Constant buffer structure
struct RotationMatrix
{
    matrix transform;
};


ConstantBuffer<RotationMatrix> RotationMatrixBuffer : register(b0);

// Vertex shader function
VertexOutput main(float3 pos : POSITION, float3 normal : NORMAL, float3 tangent : TANGENT, float2 uv : TEXCOORD)
{
    VertexOutput output;

    output.Color = float4(normal, 1.0f);
    const float4 posPoint = float4(pos.xyz, 1.0f);


    output.position = mul(RotationMatrixBuffer.transform, posPoint);

    return output;
}