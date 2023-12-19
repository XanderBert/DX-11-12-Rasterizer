//global variables
//
float4x4 gWorldViewProj : WorldViewProjection;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;


struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TexCoord : TEXCOORD;
};



struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
	float2 TexCoord : TEXCOORD;
};
//------------------------------------------------
// Vertex Shader
//------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	//multiply the World View Projection Matrix with every vertex position
	output.Position = float4(mul(float4(input.Position, 1.0f), gWorldViewProj));
	output.TexCoord = input.TexCoord;
	output.Color = input.Normal;
	return output;
}


//
//SamplerState gSampler
//Register 0
SamplerState samPoint : register(s0)
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

//------------------------------------------------
// Pixel Shader
//------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(samPoint, input.TexCoord);
}



//------------------------------------------------a
// Technique
//------------------------------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
		
	}
}