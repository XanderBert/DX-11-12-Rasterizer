#pragma once
#include "PosCol3DEffect.h"

class TextureEffect : public PosCol3DEffect
{
public:
    TextureEffect(ID3D11Device* pDevice, const std::wstring& assetFile, const LPCSTR& techniqueName);
    ~TextureEffect() override = default;

    TextureEffect(const TextureEffect&) = delete;
    TextureEffect(TextureEffect&&) noexcept = delete;
    TextureEffect& operator=(const TextureEffect&) = delete;
    TextureEffect& operator=(TextureEffect&&) noexcept = delete;
    
    void SetDiffuseMap(ID3D11ShaderResourceView* pResourceView) const;

protected:
    ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{ nullptr };
};
