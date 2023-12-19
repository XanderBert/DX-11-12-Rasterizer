#pragma once
#include "IEffect.h"

class PosCol3DEffect : public IEffect
{
public :
    PosCol3DEffect(ID3D11Device* pDevice, const std::wstring& assetFile, const LPCSTR& techniqueName);
    ~PosCol3DEffect() override = default;

    PosCol3DEffect(const PosCol3DEffect&) = delete;
    PosCol3DEffect(PosCol3DEffect&&) noexcept = delete;
    PosCol3DEffect& operator=(const PosCol3DEffect&) = delete;
    PosCol3DEffect& operator=(PosCol3DEffect&&) noexcept = delete;

    void SetWorldViewProjectionMatrix(const dae::Matrix* worldViewProjectionMatrix) const;

protected:
    ID3DX11EffectMatrixVariable* m_pWorldViewProjectionVar{ nullptr };
};

