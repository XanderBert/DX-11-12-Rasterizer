#pragma once
#include "IEffect.h"

/**
 * \brief basic Shader with a WorldViewProjection matrix using a vertex color
 */
class Effect final : public IEffect
{
public:
    Effect(ID3D11Device* pDevice, const std::wstring& assetFile, const LPCSTR& techniqueName);
    ~Effect() override = default;

    Effect(const Effect&) = delete;
    Effect(Effect&&) noexcept = delete;
    Effect& operator=(const Effect&) = delete;
    Effect& operator=(Effect&&) noexcept = delete;
    
    void SetWorldViewProjectionMatrix(const dae::Matrix* worldViewProjectionMatrix) const;

private:
    ID3DX11EffectMatrixVariable* m_pWorldViewProjectionVar{ nullptr };
};
