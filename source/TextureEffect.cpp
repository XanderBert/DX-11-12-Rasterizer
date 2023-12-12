#include "pch.h"
#include "TextureEffect.h"

TextureEffect::TextureEffect(ID3D11Device* pDevice, const std::wstring& assetFile, const LPCSTR& techniqueName) : PosCol3DEffect(pDevice, assetFile, techniqueName)
{
    m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
    assert(m_pDiffuseMapVariable->IsValid() && "TextureEffect::TextureEffect() -> GetVariableByName() not valid!");
}

void TextureEffect::SetDiffuseMap(ID3D11ShaderResourceView* pResourceView) const
{
    assert(pResourceView && "TextureEffect::SetDiffuseMap() -> pResourceView is nullptr!");

    if(pResourceView)
    m_pDiffuseMapVariable->SetResource(pResourceView);
}
