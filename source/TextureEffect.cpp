#include "pch.h"
#include "TextureEffect.h"

TextureEffect::TextureEffect(ID3D11Device* pDevice, const std::wstring& assetFile, const LPCSTR& techniqueName)
: PosCol3DEffect(pDevice, assetFile, techniqueName)
, m_UseSpecularBool(m_pEffect, "gUseSpecularPhong", true)
, m_UseHalfLambertBool(m_pEffect, "gUseHalfLambert", false)
, m_UseNormalMapBool(m_pEffect, "gUseTextureNormal", true)
, m_LightDirectionVector(m_pEffect, "gLightDirection", dae::Vector3{0.577f,-0.577f,0.577f})
, m_AmbientColorVector(m_pEffect, "gAmbientColor", dae::Vector3{0.03f,0.03f,0.03f})
, m_LightColorVector(m_pEffect, "gLightColor", dae::Vector3{1.f,1.f,1.f})
, m_FlipGreenChannelBool(m_pEffect, "gFlipGreenChannel", false)
, m_RemapNormalRangeBool(m_pEffect, "gRemapNormal", false)
, m_UseCookTorranceBool(m_pEffect, "gUseCookTorrance", false)
, m_CameraPositionVector(m_pEffect, "gCameraPosition", dae::Vector3{0.f,0.f,-50.f})
, m_ShininessFloat(m_pEffect, "gShininess", 25.f)
, m_LightIntensityFloat(m_pEffect, "gLightIntensity", 7.f)
, m_ContrastFloat(m_pEffect, "gContrast", 1.f)
, m_TimeFloat(m_pEffect, "gTime", 0.f)
, m_UseCombustionModulation(m_pEffect, "gCombustionModulation", true)


{
    //Diffuse map
    m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
    assert(m_pDiffuseMapVariable->IsValid() && "TextureEffect::TextureEffect() -> GetVariableByName() not valid!");

    //Normal map
    m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
    assert(m_pNormalMapVariable->IsValid() && "TextureEffect::TextureEffect() -> GetVariableByName() not valid!");

    //Specular map
    m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
    assert(m_pSpecularMapVariable->IsValid() && "TextureEffect::TextureEffect() -> GetVariableByName() not valid!");

    //Glossiness map
    m_pGlossinessVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
    assert(m_pGlossinessVariable->IsValid() && "TextureEffect::TextureEffect() -> GetVariableByName() not valid!");

    m_pPartialCoverageMapVariable = m_pEffect->GetVariableByName("gFireEffectMap")->AsShaderResource();
    assert(m_pPartialCoverageMapVariable->IsValid() && "TextureEffect::TextureEffect() -> GetVariableByName() not valid!");
    
    m_pWorldMatrixVariable = m_pEffect->GetVariableByName("gWorldmatrix")->AsMatrix();
    assert(m_pWorldMatrixVariable->IsValid() && "Effect::Effect() -> gWorldmatrix variable not valid!");

    m_pSamplerVariable = m_pEffect->GetVariableByName("gSampler")->AsSampler();
    assert(m_pSamplerVariable->IsValid() && "Effect::Effect() -> gSampler variable not valid!");

   
    
    //Sampler state
    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = SamplerManager::GetCurrentSamplerDirectXType();
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; //D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; //D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; //D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    
    Microsoft::WRL::ComPtr<ID3D11SamplerState> pSamplerState{ nullptr };
    
    HRESULT hr = pDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
    assert(SUCCEEDED(hr) && "TextureEffect::TextureEffect() -> CreateSamplerState() failed!");
    
    
    m_pSamplerVariable->SetSampler(0, pSamplerState.Get());
}

TextureEffect::~TextureEffect()
{
    SafeRelease(m_pDiffuseMapVariable)
    SafeRelease(m_pNormalMapVariable)
    SafeRelease(m_pSpecularMapVariable)
    SafeRelease(m_pGlossinessVariable)
    SafeRelease(m_pPartialCoverageMapVariable)
    SafeRelease(m_pWorldMatrixVariable)
}


void TextureEffect::SetTextureMap(TextureType type, ID3D11ShaderResourceView* pResourceView) const
{
    assert(pResourceView && "TextureEffect::SetTextureMap() -> pResourceView is nullptr!");

    if(pResourceView)
    {
        switch(type)
        {
        case TextureType::Diffuse:
            m_pDiffuseMapVariable->SetResource(pResourceView);
            break;
        case TextureType::Normal:
            m_pNormalMapVariable->SetResource(pResourceView);
            break;
        case TextureType::Specular:
            m_pSpecularMapVariable->SetResource(pResourceView);
            break;
        case TextureType::Glossiness:
            m_pGlossinessVariable->SetResource(pResourceView);
            break;
        case TextureType::PartialCoverage:
            m_pPartialCoverageMapVariable->SetResource(pResourceView);
            break;
        }
    }
}

void TextureEffect::Update(float totalTime, const dae::Matrix* worldMatrix, const dae::Matrix* worldViewProjectionMatrix, const dae::Vector3& cameraPosition)
{
    SetWorldMatrix(worldMatrix);
    SetCameraPosition(cameraPosition);
    SetWorldViewProjectionMatrix(worldViewProjectionMatrix);
    //Update Time    
    m_TimeFloat.Set(totalTime);
}

void TextureEffect::SetWorldMatrix(const dae::Matrix* worldMatrix) const
{
    m_pWorldMatrixVariable->SetMatrix(reinterpret_cast<const float*>(worldMatrix));
}

void TextureEffect::SetCameraPosition(const dae::Vector3& cameraPosition)
{
    m_CameraPositionVector.Set(cameraPosition);
}

void TextureEffect::IncrementFilter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    assert(pDevice != nullptr && "Mesh::IncrementFilter() -> pDevice is nullptr!");
    assert(pDeviceContext != nullptr && "Mesh::IncrementFilter() -> pDeviceContext is nullptr!");
    
    //Sampler state
    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter =   SamplerManager::NextSamplerType();
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; //D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; //D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; //D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    Microsoft::WRL::ComPtr<ID3D11SamplerState> pSamplerState{ nullptr };
    HRESULT hr = pDevice->CreateSamplerState(&samplerDesc, &pSamplerState);
    assert(SUCCEEDED(hr) && "TextureEffect::TextureEffect() -> CreateSamplerState() failed!");
    
    
    m_pSamplerVariable->SetSampler(0, pSamplerState.Get());
}

std::string TextureEffect::GetCurrentSamplerType()
{
    return SamplerManager::GetCurrentSamplerType();
}

std::string TextureEffect::GetUseNormalString()
{
    return m_UseNormalMapBool.Get() ? "Normal Map" : "No Normal Map";
}

std::string TextureEffect::GetUseSpecularString()
{
    return m_UseSpecularBool.Get() ? "Specular-Phong" : "Blinn-Phong";
}

std::string TextureEffect::GetHalfLambertString()
{
    return m_UseHalfLambertBool.Get() ? "Half Lambert" : "Lambert";
}

std::string TextureEffect::GetFlipGreenChannelString()
{
    return m_FlipGreenChannelBool.Get() ? "Flipped Green Channel" : "Normal Green Channel";
}

std::string TextureEffect::GetRemapNormalRangeString()
{
    return m_RemapNormalRangeBool.Get() ? "Normal Range: [-1,1]" : "Normal Range: [0, 1]";
}

std::string TextureEffect::GetUseCookTorranceString()
{
    return m_UseCookTorranceBool.Get() ? "Cook-Torrance" : "Phong";
}

std::string TextureEffect::GetUseCombustionModulationString()
{
    return m_UseCombustionModulation.Get() ? "On" : "Off";
}

