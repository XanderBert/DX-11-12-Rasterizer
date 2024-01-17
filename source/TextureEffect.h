#pragma once
#include <functional>
#include <map>

#include "PosCol3DEffect.h"

enum class TextureType : uint8_t
{
    Diffuse,
    Normal,
    Specular,
    Glossiness,
    PartialCoverage
};

enum class SamplerType : uint8_t
{
    Point,
    Linear,
    Anisotropic
};


class SamplerManager
{
public:
    static D3D11_FILTER GetFilter(SamplerType samplerType)
    {
        return samplerTypes.at(samplerType);
    }

    static D3D11_FILTER NextSamplerType()
    {
        currentSamplerType = static_cast<SamplerType>((static_cast<uint8_t>(currentSamplerType) + 1) % samplerTypes.size());
        return GetCurrentSamplerDirectXType();
    }

    static D3D11_FILTER GetCurrentSamplerDirectXType()
    {
        return samplerTypes[currentSamplerType];
    }

private:
    inline static std::map<SamplerType, D3D11_FILTER> samplerTypes =
        {
        {SamplerType::Point, D3D11_FILTER_MIN_MAG_MIP_POINT},
        {SamplerType::Linear, D3D11_FILTER_MIN_MAG_MIP_LINEAR},
        {SamplerType::Anisotropic, D3D11_FILTER_ANISOTROPIC}
        };

    inline static SamplerType currentSamplerType;
};


class TextureEffect : public PosCol3DEffect
{
public:
    TextureEffect(ID3D11Device* pDevice, const std::wstring& assetFile, const LPCSTR& techniqueName);
    ~TextureEffect() override;

    TextureEffect(const TextureEffect&) = delete;
    TextureEffect(TextureEffect&&) noexcept = delete;
    TextureEffect& operator=(const TextureEffect&) = delete;
    TextureEffect& operator=(TextureEffect&&) noexcept = delete;

    void SetTextureMap(TextureType type, ID3D11ShaderResourceView* pResourceView) const;

    void SetViewInverseMatrix(const dae::Matrix* viewInverseMatrix) const;
    void SetWorldMatrix(const dae::Matrix* worldMatrix) const;
    void SetCameraPosition(const dae::Vector3* cameraPosition) const;

    void IncrementFilter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
    
protected:
    //TODO: There should be a way to just add and remove textures instead of having a fixed set of textures
    //This means i should get rid of the static enum class TextureType and find a dynamic way to bind types with textures 
    ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{ nullptr };
    ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{ nullptr };
    ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{ nullptr };
    ID3DX11EffectShaderResourceVariable* m_pGlossinessVariable{ nullptr };
    ID3DX11EffectShaderResourceVariable* m_pPartialCoverageMapVariable{ nullptr };
    
    ID3DX11EffectMatrixVariable* m_pViewInverseMatrixVariable{ nullptr };
    ID3DX11EffectMatrixVariable* m_pWorldMatrixVariable{ nullptr };

    //Try this instead of the viewInverse;
    ID3DX11EffectVectorVariable* m_pCameraPositionVariable{ nullptr };

    //Sampler state:
    ID3DX11EffectSamplerVariable* m_pSamplerVariable{ nullptr };
    ID3D11SamplerState* m_pSamplerState{ nullptr };
};
