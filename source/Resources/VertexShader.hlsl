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

    output.Color = float4(uv, 1.f, 1.0f);
    output.position = mul(float4(pos.xyz,1), RotationMatrixBuffer.transform);

    return output;
}