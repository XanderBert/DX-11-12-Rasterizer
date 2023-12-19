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
        }
    }
}