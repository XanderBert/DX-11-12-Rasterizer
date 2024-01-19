//Static variables
const static float gPi  = 3.14159265359;
const static bool gUseTextureSpecularIntensity = true;
const static float3 gColorSpecular = float3(1,1,1);

//ImGui Variables
extern float gTime = 0.0;
extern float gContrast = float(1.0);
extern bool gCombustionModulation = true;
extern bool gUseHalfLambert = true;
extern bool gUseSpecularPhong = true;
extern bool gUseTextureNormal = true;
extern bool gFlipGreenChannel = false;
extern bool gRemapNormal = false;
extern bool gUseCookTorrance = false;
extern float gShininess = 25.0f;
extern float gLightIntensity : LightIntensity = float(7.0);
extern float3 gLightDirection : LightDirection = float3(0.577f, -0.577f, 0.577f);
extern float3 gAmbientColor : AmbientColor = float3(0.03, 0.03, 0.03);
extern float3 gLightColor : LightColor = float3(1,1,1);

//Matrices
extern float4x4 gWorldViewProj : WorldViewProjection;
extern float4x4 gWorldmatrix : WorldMatrix;
extern float3 gCameraPosition : Camera;

//Should be grouped in a constant buffer
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


SamplerState gSampler : register(s0)
{ 
	Filter = MIN_MAG_MIP_LINEAR;
 	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
};

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
   	float3 halfVector = normalize(viewDirection + normalize(gLightDirection));
	
	//Calculate the specular value
	float specularValue = saturate(dot(normal, halfVector));

	//Return SpecularValue
	return float3(specularValue, specularValue, specularValue);
}

float3 CalculateSpecularPhong(float3 viewDirection, float3 normal)
{	
	//Calculate the refelection
	float3 reflectionDirection = reflect(-normalize(gLightDirection), normal);
	
	//Calculate the specular value
	float specularValue = saturate(dot(viewDirection, reflectionDirection));
	
	//Return specularValue
	return float3(specularValue, specularValue, specularValue);
}

float3 CalculateSpecular(float3 viewDirection, float3 normal, float2 texCoord)
{	
	float3 specularValue = float3(0, 0, 0);

	if(gUseSpecularPhong)
	{
		specularValue = CalculateSpecularPhong(viewDirection, normal);
	}else
	{
 		specularValue = CalculateSpecularBlinn(viewDirection, normal);
	}


	//Tone Down on the refelctions with the power of the glossiness map
	float specularExp = gShininess * gGlossinessMap.Sample(gSampler, texCoord).r;
  	float3 specularPower = pow(specularValue, float3(specularExp, specularExp, specularExp));
	
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

		//remap the normals in range [-1,1] if needed
		if(gRemapNormal)
		{
			newNormal.x = 2 * normalMap.x - 1;
			newNormal.y = 2 * normalMap.y - 1;
			newNormal.z = 2 * normalMap.z - 1;	
		}
		else
		{
			newNormal = normalMap;
		}
	
		//flip the x axis if needed
		if(gFlipGreenChannel) newNormal.x = -newNormal.x;
	}

	return newNormal;
}


float3 CalculateCookTorrance(float3 normal, float3 viewDir, float3 lightDir, float roughness, float metallic, float3 albedo, float3 F0)
{
	float3 H = normalize(lightDir + viewDir);
	float NdotH = max(dot(normalize(normal), H), 0.0);
	float NdotV = max(dot(normalize(normal), normalize(viewDir)), 0.0);
	float NdotL = max(dot(normalize(normal), normalize(lightDir)), 0.0);
	float VdotH = max(dot(normalize(viewDir), H), 0.0);

    // Geometric attenuation
    float a = roughness * roughness;
    float a2 = a * a;
    float G = 2.0 / (1.0 + sqrt(1.0 + a2 * (1.0 - NdotV * NdotV) / (NdotV * NdotV)));

    // Roughness (or microfacet distribution)
    float D = exp((NdotH * NdotH - 1.0) / (a2 * NdotH * NdotH)) / (gPi * a2 * NdotH * NdotH * NdotH * NdotH);

    // Fresnel Schlick 
    float3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

    // Specular
    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);
    float3 spec = (D * G * F) / (4.0 * NdotL * NdotV + 0.001); // prevent divide by zero

    // Add to diffuse the light
    float3 diffColor = kD * albedo / gPi;
    float3 specColor = spec;

    return saturate(diffColor * gLightIntensity + specColor);
}

float3 CalculateDiffuse(float3 normal, float2 texCoord, float3 viewDirection)
{
	//Get the diffuse color from the diffuse map
	float3 diffColor = gDiffuseMap.Sample(gSampler, texCoord).rgb;
	

	if(gUseCookTorrance)
	{
		// Get the roughness, metallic, and ao from their respective maps
    	//float roughness = gRoughnessMap.Sample(gSampler, texCoord).r;		
		//float metallic = 0.5;
		float roughness = gGlossinessMap.Sample(gSampler, texCoord).r;
		float metallic = 0.0;

    	//float ao = gAOMap.Sample(gSampler, texCoord).r;

		// Calculate the Cook-Torrance BRDF
		float3 F0 = float3(0.04, 0.04, 0.04); // F0 for dielectrics
		float3 bdrf = CalculateCookTorrance(normal, viewDirection, normalize(gLightDirection), roughness, metallic, diffColor, F0);
		bdrf *= normalize(gLightColor);
		//bdrf *= ao;

		return bdrf;
	}

	
	//Calculate the strenght with the angle beween light and normal
	//use saturate() to clamp that value between [0, 1]
	float diffuseStrength = saturate(dot(normal, normalize(gLightDirection)));
	
	//Calculate the half lambert
	float3 lambert = pow(diffuseStrength * 0.5 + 0.5, 2.0);
	if(!gUseHalfLambert) lambert = diffuseStrength / gPi;
	
	//Calculate the diffuse color
    diffColor = diffColor * lambert;
	
	return diffColor * gLightIntensity + gAmbientColor;
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
	finalColor = saturate(pow(abs(finalColor), gContrast));
	return float4(finalColor, 1.f);
}


//------------------------------------------------
// Flat Pixel Shader
//------------------------------------------------
float4 PSFlat(VertexShaderOutput pixelShaderInput) : SV_TARGET
{
	//Just sample the texture and return it
	float4 finalColor =  gFireEffectMap.Sample(gSampler, pixelShaderInput.TexCoord);


	float contrast = gContrast;
	if(gCombustionModulation)
	{
		contrast += (sin(gTime * 8.f)) / 1.4f;
	}

	finalColor = pow(abs(finalColor), contrast);

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
