//global variables
//
bool gUseSpecularPhong = true;
bool gUseSpecularBlinn = false;
bool gUseTextureNormal = true;
bool gFlipGreenChannel = false;
bool gUseTextureSpecularIntensity = true;
float3 gColorSpecular = float3(1,1,1);

float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WorldMatrix;

float3 gCameraPos : CameraPosition;

float gPi  = 3.1415;
float gShininess = 1.0f;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float3 gLightDirection : LightDirection = float3(0.577f, -0.577f, 0.577f);

//SamplerState gSampler
//Register 0
SamplerState samPoint : register(s0)
{
	Filter = MIN_MAG_MIP_LINEAR;
 	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
};


struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TexCoord : TEXCOORD0;
};



struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Normal: NORMAL;
	float3 Tangent: TANGENT;
	float2 TexCoord : TEXCOORD0;
};


float3 CalculateSpecularBlinn(float3 viewDirection, float3 normal, float2 texCoord)
{
	//H = normalize(ViewDirection + LightDirection)
	//N = normal
	//Specular = |H| * |N| * Cos(Theta) = Dot(H, N)
	
	//Calculate the halfvector
   	float3 halfVector = normalize(viewDirection + gLightDirection);
	
	//Calculate the Specular value and use max() to keep it positive.
	float specularValue = max(dot(normal, halfVector), 0.0);
	
	//Return SpecularValue
	return float3(specularValue, specularValue, specularValue);
}

float3 CalculateSpecularPhong(float3 viewDirection, float3 normal, float2 texCoord)
{	
	//Calculate the refelection
	float3 reflectionDirection = reflect(-gLightDirection, normal);
	
	//Calculate the specularValue value and use max() to keep it positive.
	float specularValue = max(dot(reflectionDirection, viewDirection), 0.0);
	
	//Return SpecularValue
	return float3(specularValue, specularValue, specularValue);
}

float3 CalculateSpecular(float3 viewDirection, float3 normal, float2 texCoord)
{	
	float3 specularValue = float3(0,0,0);

	if(gUseSpecularPhong) specularValue = CalculateSpecularPhong(viewDirection, normal, texCoord);
	if(gUseSpecularBlinn) specularValue = CalculateSpecularBlinn(viewDirection, normal, texCoord);
	
	//Tone Down on the refelctions with the power
  	float specularPower = pow(specularValue, gShininess);
	
	if(gUseTextureSpecularIntensity)
	{
		float3 specTexture = gSpecularMap.Sample(samPoint, texCoord);
		
		//Return the Specular
		return gColorSpecular * specularPower * specTexture;
	}
	
	//Return the Specular
	return gColorSpecular * specularPower;
}

float3 CalculateNormal(float3 tangent, float3 normal, float2 texCoord)
{
	float3 newNormal = normal;
		
	if(gUseTextureNormal)
	{
		float3 normalMap = gNormalMap.Sample(samPoint, texCoord);
	
		newNormal.x = 2 * normalMap.x - 1;
		newNormal.y = 2 * normalMap.y - 1;
		newNormal.z = 2 * normalMap.z - 1;
		
		if(gFlipGreenChannel) newNormal.x = -newNormal.x;
	}

	return newNormal;
}




float3 CalculateDiffuse(float3 normal, float2 texCoord)
{
	//Get the diffuse color from the diffuse map
	float3 diffColor = gDiffuseMap.Sample(samPoint, texCoord);
	
	//Calculate the strenght with the angle beween light and normal
	//use saturate() to clamp that value between [0, 1]
	float diffuseStrength = saturate(dot(normal, gLightDirection));
	
	//Calculate the half lambert
	float3 lambert = pow(diffuseStrength * 0.5 + 0.5, 2.0);
	
	//If we won't use a half lambert set lambert to the strength so we get a normal lambert
	//if(!gUseHalfLambert) lambert = diffuseStrength;
	
	//Calculate the diffuse color
    diffColor = diffColor * lambert;
	
	return diffColor;
}





//------------------------------------------------
// Vertex Shader
//------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	
	//multiply the World View Projection Matrix with every vertex position
	output.Position = float4(mul(float4(input.Position, 1.0f), gWorldViewProj));

	//TODO: MULTIPLY WITH WORLD NOT WITH WORLDVIEWPROJ
	output.Normal = mul(input.Normal, (float3x3)gWorldViewProj);
	output.Tangent = mul(input.Tangent, (float3x3)gWorldViewProj);
	output.TexCoord = input.TexCoord;


	return output;
}




//------------------------------------------------
// Pixel Shader
//------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	// NORMALIZE
	input.Normal = normalize(input.Normal);
	input.Tangent = normalize(input.Tangent);

	//float3 viewDirectionMinus = normalize(input.WorldPosition.xyz - gMatrixViewInverse[3].xyz);
	//float3 viewDirection = -viewDirectionMinus;
	float3 viewDirection = float3(1,0,1);
	

	//NORMAL
	float3 newNormal = CalculateNormal(input.Tangent, input.Normal, input.TexCoord);

	//SPECULAR
	float3 specColor = CalculateSpecular(viewDirection, newNormal, input.TexCoord);
	
		
		
	//DIFFUSE
	float3 diffColor = CalculateDiffuse(newNormal, input.TexCoord);

	//FINAL COLOR
	float3 finalColor = diffColor + specColor;

	return float4(finalColor, 1.f);
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