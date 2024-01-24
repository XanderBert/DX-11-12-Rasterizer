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
   	float3 halfVector = normalize(viewDirection + normalize(-gLightDirection));
	
	//Calculate the specular value
	float specularValue = saturate(dot(normal, halfVector));

	//Return SpecularValue
	return float3(specularValue, specularValue, specularValue);
}

float3 CalculateSpecularPhong(float3 viewDirection, float3 normal)
{	
	//Calculate the refelection
	float3 reflectionDirection = reflect(normalize(gLightDirection), normal);
	
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
		const float3 binormal = cross(normal, tangent);
		const float4x4 tangentSpaceAxis = float4x4(float4(tangent, 0.0f), float4(binormal, 0.0f), float4(normal, 0.0), float4(0.0f, 0.0f, 0.0f, 1.0f));

		float3 normalMap = normalize(gNormalMap.Sample(gSampler, texCoord).rgb);


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

		newNormal = mul(float4(newNormal, 0.0f), tangentSpaceAxis);
	}

	return newNormal;
}


float3 CalculateCookTorrance(float3 normal, float3 viewDir, float3 lightDir, float roughness, float metallic, float3 albedo, float3 F0, float3 specularColor)
{
	float3 n = normalize(normal);
	float3 ld = normalize(lightDir);
	float3 vd = normalize(viewDir);
	float3 H = normalize(ld + vd);

	float NdotL = saturate(dot(n, ld));
	float NdotV = saturate(dot(n, vd));
	float NdotH = saturate(dot(n, H));
	float LdotH = saturate(dot(ld, H));

	float NdotHSqr = NdotH * NdotH;

	// Roughness
    float roughness2   = roughness * roughness;
    float roughnessSqr = roughness2 * roughness2;


	//Nayar
	//https://en.wikipedia.org/wiki/Oren%E2%80%93Nayar_reflectance_model
	//Oren-Nayar BRDF is an improvement over the theoretical Lambertian model, applying distribution of purely diffuse microfacets.
	float A = 1.0 - 0.5 * (roughnessSqr / (roughnessSqr + 0.33));
    float B = 0.45 * (roughnessSqr / (roughnessSqr + 0.09));
    float C = saturate(dot(normalize(vd - n * NdotV), normalize(ld - n * NdotL)));
    float angleL = acos(NdotL);
    float angleV = acos(NdotV);
    float alpha  = max(angleL, angleV);
    float beta   = min(angleL, angleV);
    float3 diffuse = saturate(albedo.rgb * (A + B * C * sin(alpha) * tan(beta)) * gLightColor.rgb * NdotL);


	// Cook-Torrance
    float D = roughnessSqr / (gPi * pow(NdotHSqr * (roughnessSqr - 1.0) + 1.0, 2.0));
    float k  = roughness2 * 0.5;
	float gl = NdotL / (NdotL * (1.0 - k) + k);
    float gv = NdotV / (NdotV * (1.0 - k) + k);
    float G = gl * gv;
    float F = metallic + (1.0 - metallic) * pow(1.0 - LdotH, 5.0);
    float3 specular = saturate((D * G * F) / (4.0 * NdotV)) * gPi * gLightColor.rgb;

	//Use the specularMap 
	specular = saturate(specular * specularColor);


	float3 color = (lerp(diffuse, specular, metallic));

	color *= gLightIntensity;
	color += gAmbientColor;

	return (color);
}

float3 CalculateDiffuse(float3 normal, float2 texCoord, float3 viewDirection, float3 specularColor)
{
	//Get the diffuse color from the diffuse map
	float3 diffColor = gDiffuseMap.Sample(gSampler, texCoord).rgb;
	

	if(gUseCookTorrance)
	{
		// Get the roughness, metallic, and ao from their respective maps
		float roughness = gGlossinessMap.Sample(gSampler, texCoord).r;


		//Should be 0 or 1 in a map but i don't have a map. So i just set it to 0.85
		float metallic = 0.85;
    	//float ao = gAOMap.Sample(gSampler, texCoord).r;

		// Calculate the Cook-Torrance BRDF
		float3 F0 = float3(0.98, 0.82, 0.76); // F0 for dielectrics
		float3 bdrf = CalculateCookTorrance(normal, viewDirection, -gLightDirection, roughness, metallic, diffColor, F0, specularColor);
		bdrf *= normalize(gLightColor);
		//bdrf *= ao;

		return bdrf;
	}

	
	//Calculate the strenght with the angle beween light and normal
	//use saturate() to clamp that value between [0, 1]
	float diffuseStrength = saturate(dot(normal, normalize(-gLightDirection)));
	
	//Calculate the half lambert
	float3 lambert = pow(diffuseStrength * 0.5 + 0.5, 2.0);
	if(!gUseHalfLambert) lambert = diffuseStrength / gPi;
	
	//Calculate the diffuse color
    diffColor = diffColor * lambert;
	
	return diffColor * gLightIntensity + gAmbientColor + specularColor;
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

//------------------------------------------------
// Pixel Shader
//------------------------------------------------
float4 PS(VertexShaderOutput pixelShaderInput) : SV_TARGET
{
	// NORMALIZE
	pixelShaderInput.Normal = normalize(pixelShaderInput.Normal);
	pixelShaderInput.Tangent = normalize(pixelShaderInput.Tangent);

	float3 viewDirectionMinus = normalize(pixelShaderInput.WorldPosition.xyz - gCameraPosition.xyz);
	float3 viewDirection = -viewDirectionMinus;
	
	//NORMAL
	float3 newNormal = CalculateNormal(pixelShaderInput.Tangent, pixelShaderInput.Normal, pixelShaderInput.TexCoord);

	//SPECULAR
	float3 specColor = CalculateSpecular(viewDirection, newNormal, pixelShaderInput.TexCoord);
			
	//DIFFUSE + SPECULAR
	float3 diffColor = CalculateDiffuse(newNormal, pixelShaderInput.TexCoord,  viewDirection, specColor);

	//FINAL COLOR
	float3 finalColor = diffColor;
	
	//add more contrast
	finalColor = saturate(pow(abs(finalColor), gContrast));

	return float4(finalColor, 1.0);
}


//------------------------------------------------
// Flat Pixel Shader
//------------------------------------------------
float4 PSFlat(VertexShaderOutput pixelShaderInput) : SV_TARGET
{
	//Just sample the texture and return it
	float4 finalColor =  gFireEffectMap.Sample(gSampler, pixelShaderInput.TexCoord);
	//finalColor = saturate(float4(gAmbientColor, 1.0f) + finalColor);

	float contrast = gContrast;
	if(gCombustionModulation)
	{
		//Making the less predicible by making a wave like pattern
		//Idea from: https://developer.nvidia.com/gpugems/gpugems2/part-ii-shading-lighting-and-shadows/chapter-18-using-vertex-texture-displacement
		float wave1 = (sin(gTime * 8.f)) * 2.0f;
		float wave2 = (sin(gTime * 4.f + wave1)) / 1.5f;
		float wave3 = (sin(gTime * 8.f + wave2)) / 1.2f;

		contrast += wave3;
	}

	finalColor = saturate(pow(abs(finalColor), max(contrast, 0.0f)));

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
