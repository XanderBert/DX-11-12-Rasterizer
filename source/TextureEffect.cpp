#include "pch.h"
#include "TextureEffect.h"

TextureEffect::TextureEffect(ID3D11Device* pDevice, const std::wstring& assetFile, const LPCSTR& techniqueName) : PosCol3DEffect(pDevice, assetFile, techniqueName)
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
    
    m_pCameraPositionVariable = m_pEffect->GetVariableByName("gCameraPosition")->AsVector();
    assert(m_pCameraPositionVariable->IsValid() && "Effect::Effect() -> gCameraPosition variable not valid!");
    
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
    SafeRelease(m_pCameraPositionVariable)
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

void TextureEffect::SetWorldMatrix(const dae::Matrix* worldMatrix) const
{
    m_pWorldMatrixVariable->SetMatrix(reinterpret_cast<const float*>(worldMatrix));
}

void TextureEffect::SetCameraPosition(const dae::Vector3* cameraPosition) const
{
    m_pCameraPositionVariable->SetFloatVector(reinterpret_cast<const float*>(cameraPosition));
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


