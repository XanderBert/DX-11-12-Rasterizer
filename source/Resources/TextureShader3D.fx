//Static variables
const static bool gUseSpecularPhong = true;
const static bool gUseSpecularBlinn = false;
const static bool gUseTextureSpecularIntensity = true;
const static float3 gColorSpecular = float3(1,1,1);

const static bool gUseTextureNormal = true;
const static bool gFlipGreenChannel = true;

const static bool gUseHalfLambert = true;
const static bool gUseCookTorrance = false;

const static float3 gLightDirection : LightDirection = float3(0.577f, -0.577f, 0.577f);
const static float3 gLightColor : LightColor = float3(1,1,1);
const static float gPi  = 3.14159265359;
const static float gShininess = 1.25f;


//Should be grouped in a constant buffer
uniform extern float4x4 gViewInverseMatrix : ViewInverseMatrix;
uniform extern float4x4 gWorldViewProj : WorldViewProjection;
uniform extern float4x4 gWorldmatrix : WorldMatrix;
uniform extern float3 gCameraPosition : Camera;

//Textures
uniform extern Texture2D gDiffuseMap : DiffuseMap;
uniform extern Texture2D gNormalMap : NormalMap;
uniform extern Texture2D gSpecularMap : SpecularMap;
uniform extern Texture2D gGlossinessMap : GlossinessMap;
uniform extern Texture2D gFireEffectMap : FireEffectMap;


RasterizerState gNoCulling
{
	CullMode = none;
	FrontCounterClockwise = false;
};

RasterizerState gCulling
{
	CullMode = back;
	FrontCounterClockwise = false;
};

BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};


SamplerState gSampler : register(s0){};

DepthStencilState gNoDepthStencil
{
    DepthEnable = true;
    DepthWriteMask = zero;
    DepthFunc = less;
    StencilEnable = false;
};

DepthStencilState gDepthStencil
{
	DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = less;

    StencilEnable = false;
    StencilReadMask = 0xFF;
    StencilWriteMask = 0xFF;

    FrontFaceStencilFunc = always;
    BackFaceStencilFail = keep;
    FrontFaceStencilDepthFail = keep;
    FrontFaceStencilPass = keep;

    BackFaceStencilFunc = always;
    BackFaceStencilFail = keep;
    BackFaceStencilDepthFail = keep;
    BackFaceStencilPass = keep;
};

struct VertexShaderInput
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 TexCoord : TEXCOORD0;
};



struct VertexShaderOutput
{
	float4 Position : SV_POSITION;
	float4 WorldPosition: POSITION;
	float3 Normal: NORMAL;
	float3 Tangent: TANGENT;
	float2 TexCoord : TEXCOORD0;
};



float3 CalculateSpecularBlinn(float3 viewDirection, float3 normal)
{
	//H = normalize(ViewDirection + LightDirection)
	//N = normal
	//Specular = |H| * |N| * Cos(Theta) = Dot(H, N)
	
	//Calculate the halfvector
   	float3 halfVector = normalize(viewDirection + gLightDirection);
	
	//Calculate the specular value
	float specularValue = saturate(dot(normal, halfVector));
	
	//Return SpecularValue
	return float3(specularValue, specularValue, specularValue);
}

float3 CalculateSpecularPhong(float3 viewDirection, float3 normal)
{	
	//Calculate the refelection
	float3 reflectionDirection = reflect(-gLightDirection, normal);
	
	//Calculate the specular value
	float specularValue = saturate(dot(reflectionDirection, viewDirection));
	
	//Return specularValue
	return float3(specularValue, specularValue, specularValue);
}

float3 CalculateSpecular(float3 viewDirection, float3 normal, float2 texCoord)
{	
	float3 specularValue = float3(0, 0, 0);

	if(gUseSpecularPhong) specularValue = CalculateSpecularPhong(viewDirection, normal);
	if(gUseSpecularBlinn) specularValue = CalculateSpecularBlinn(viewDirection, normal);
	
	//Tone Down on the refelctions with the power of the glossiness map
	float specularExp = gShininess * gGlossinessMap.Sample(gSampler, texCoord).r;

  	float specularPower = pow(specularValue, specularExp).r;
	
	if(gUseTextureSpecularIntensity)
	{
		float3 specTexture = gSpecularMap.Sample(gSampler, texCoord).rgb;
		
		//Return the Specular
		return normalize(gColorSpecular) * specularPower * specTexture;
	}
	
	//Return the Specular
	return normalize(gColorSpecular) * specularPower;
}

float3 CalculateNormal(float3 tangent, float3 normal, float2 texCoord)
{
	float3 newNormal = normal;
		
	if(gUseTextureNormal)
	{
		float3 normalMap = gNormalMap.Sample(gSampler, texCoord).rgb;
	
		newNormal.x = 2 * normalMap.x - 1;
		newNormal.y = 2 * normalMap.y - 1;
		newNormal.z = 2 * normalMap.z - 1;
		
		if(gFlipGreenChannel) newNormal.x = -newNormal.x;
	}

	return newNormal;
}


float3 CalculateCookTorrance(float3 normal, float3 viewDir, float3 lightDir, float roughness, float metallic, float3 albedo, float3 F0)
{
    float3 H = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, H), 0.0);
    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float VdotH = max(dot(viewDir, H), 0.0);

    // Geometric attenuation
    float a = roughness * roughness;
    float a2 = a * a;
    float G = 2.0 / (1.0 + sqrt(1.0 + a2 * (1.0 - NdotV * NdotV) / (NdotV * NdotV)));

    // Roughness (or microfacet distribution)
    float D = exp((NdotH * NdotH - 1.0) / (a2 * NdotH * NdotH)) / (3.14159 * a2 * NdotH * NdotH * NdotH * NdotH);

    // Fresnel
    float3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

    // Specular
    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);
    float3 spec = (D * G * F) / (4.0 * NdotL * NdotV + 0.001); // prevent divide by zero

    // Add to diffuse the light
    float3 diffColor = kD * albedo / 3.14159;
    float3 specColor = spec;

    return diffColor + specColor;
}

float3 CalculateDiffuse(float3 normal, float2 texCoord, float3 viewDirection)
{
	//Get the diffuse color from the diffuse map
	float3 diffColor = gDiffuseMap.Sample(gSampler, texCoord).rgb;
	

	
	if(gUseCookTorrance)
	{
		// Get the roughness, metallic, and ao from their respective maps
    	//float roughness = gRoughnessMap.Sample(gSampler, texCoord).r;
		float roughness = 0.5;
    	//float metallic = gMetallicMap.Sample(gSampler, texCoord).r;
		float metallic = 0.5;
    	//float ao = gAOMap.Sample(gSampler, texCoord).r;

		// Calculate the Cook-Torrance BRDF
		float3 F0 = float3(0.04, 0.04, 0.04); // F0 for dielectrics
		float3 bdrf = CalculateCookTorrance(normal, viewDirection, gLightDirection, roughness, metallic, diffColor, F0);
		bdrf *= normalize(gLightColor);
		//bdrf *= ao;

		return bdrf;
	}

	
	//Calculate the strenght with the angle beween light and normal
	//use saturate() to clamp that value between [0, 1]
	float diffuseStrength = saturate(dot(normal, gLightDirection));
	
	//Calculate the half lambert
	float3 lambert = pow(diffuseStrength * 0.5 + 0.5, 2.0);

	if(!gUseHalfLambert) lambert = diffuseStrength / gPi;
	
	//Calculate the diffuse color
    diffColor = diffColor * lambert;
	
	return diffColor;
}





//------------------------------------------------
// Vertex Shader
//------------------------------------------------
VertexShaderOutput VS(VertexShaderInput input)
{
	VertexShaderOutput output = (VertexShaderOutput)0;
	
	//multiply the World View Projection Matrix with every vertex position
	output.Position = float4(mul(float4(input.Position, 1.0f), gWorldViewProj));
	
	output.WorldPosition = mul(float4(input.Position, 1.0f), gWorldmatrix);
	output.Normal = mul(input.Normal, (float3x3)gWorldmatrix);
	output.Tangent = mul(input.Tangent, (float3x3)gWorldmatrix);
	output.TexCoord = input.TexCoord;


	return output;
}

void CalculateViewDirection(float3 worldPos, out float3 viewDirection)
{
	float3 invViewDirection = normalize(gCameraPosition - worldPos);
	viewDirection = -invViewDirection;
}



//------------------------------------------------
// Pixel Shader
//------------------------------------------------
float4 PS(VertexShaderOutput pixelShaderInput) : SV_TARGET
{
	// NORMALIZE
	pixelShaderInput.Normal = normalize(pixelShaderInput.Normal);
	pixelShaderInput.Tangent = normalize(pixelShaderInput.Tangent);


	float3 viewDirection;
	CalculateViewDirection(pixelShaderInput.WorldPosition.xyz, viewDirection);
	
	//NORMAL
	float3 newNormal = CalculateNormal(pixelShaderInput.Tangent, pixelShaderInput.Normal, pixelShaderInput.TexCoord);

	//SPECULAR
	float3 specColor = CalculateSpecular(viewDirection, newNormal, pixelShaderInput.TexCoord);
	
		
	//DIFFUSE
	float3 diffColor = CalculateDiffuse(newNormal, pixelShaderInput.TexCoord,  viewDirection);

	//FINAL COLOR
	float3 finalColor = diffColor + specColor;
	
	//add more contrast
	finalColor = pow(abs(finalColor), 1.8f);


	return float4(finalColor, 1.f);
}


//------------------------------------------------
// Flat Pixel Shader
//------------------------------------------------
float4 PSFlat(VertexShaderOutput pixelShaderInput) : SV_TARGET
{
	//Just sample the texture and return it
	float4 finalColor =  gFireEffectMap.Sample(gSampler, pixelShaderInput.TexCoord);

	//add more contrast
	finalColor = pow(abs(finalColor), 1.8f);

	return float4(finalColor);
}

//------------------------------------------------a
// Technique
//------------------------------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gCulling);
		SetDepthStencilState(gDepthStencil, 0);

		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 FlatPartialCoverageTechnique
{
	pass P0
	{
		SetRasterizerState(gNoCulling);
		SetDepthStencilState(gNoDepthStencil, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);


		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSFlat()));
	}
}
