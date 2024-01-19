#pragma once
#include <functional>
#include <map>

#include "Dx11EffectVariables.h"
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

    static std::string GetCurrentSamplerType()
    {
        switch (currentSamplerType)
        {
        case SamplerType::Point:
            return "Point";
        case SamplerType::Linear:
            return "Linear";
        case SamplerType::Anisotropic:
            return "Anisotropic";
        default:
            return "Unknown";
        }
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
    virtual ~TextureEffect() override;

    TextureEffect(const TextureEffect&) = delete;
    TextureEffect(TextureEffect&&) noexcept = delete;
    TextureEffect& operator=(const TextureEffect&) = delete;
    TextureEffect& operator=(TextureEffect&&) noexcept = delete;

    void SetTextureMap(TextureType type, ID3D11ShaderResourceView* pResourceView) const;

    void Update(float elapsedSec, const dae::Matrix* worldMatrix, const dae::Vector3& cameraPosition); 
    
    void IncrementFilter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
    static std::string GetCurrentSamplerType();
    
    DX11EffectVector3* GetLightDirectionVector(){return &m_LightDirectionVector;}
    DX11EffectVector3* GetAmbientColorVector(){return &m_AmbientColorVector;}
    DX11EffectVector3* GetLightColorVector(){return &m_LightColorVector;}

    std::string GetUseNormalString();
    DX11EffectBool* GetUseNormalBool() {return &m_UseNormalMapBool; }
    
    std::string GetUseSpecularString();
    DX11EffectBool* GetUseSpecularBool() {return &m_UseSpecularBool; }
    
    std::string GetHalfLambertString();
    DX11EffectBool* GetUseHalfLambertBool() {return &m_UseHalfLambertBool; }

    std::string GetFlipGreenChannelString();
    DX11EffectBool* GetFlipGreenChannelBool() {return &m_FlipGreenChannelBool; }

    std::string GetRemapNormalRangeString();
    DX11EffectBool* GetRemapNormalRangeBool() {return &m_RemapNormalRangeBool; }

    std::string GetUseCookTorranceString();
    DX11EffectBool* GetUseCookTorranceBool() {return &m_UseCookTorranceBool; }

    std::string GetUseCombustionModulationString();
    DX11EffectBool* GetUseCombustionModulationBool() {return &m_UseCombustionModulation; }
    
    DX11EffectFloat* GetShininessFloat() {return &m_ShininessFloat; }
    DX11EffectFloat* GetLightIntensityFloat() {return &m_ShininessFloat; }
    DX11EffectFloat* GetContrastFloat() {return &m_ContrastFloat; }


protected:
    void SetWorldMatrix(const dae::Matrix* worldMatrix) const;
    void SetCameraPosition(const dae::Vector3& cameraPosition);


    //TODO: There should be a way to just add and remove textures instead of having a fixed set of textures
    //This means i should get rid of the static enum class TextureType and find a dynamic way to bind types with textures 
    ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{ nullptr };
    ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{ nullptr };
    ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{ nullptr };
    ID3DX11EffectShaderResourceVariable* m_pGlossinessVariable{ nullptr };
    ID3DX11EffectShaderResourceVariable* m_pPartialCoverageMapVariable{ nullptr };
    
    ID3DX11EffectMatrixVariable* m_pWorldMatrixVariable{ nullptr };
    ID3DX11EffectSamplerVariable* m_pSamplerVariable{ nullptr };
    
    DX11EffectFloat m_TimeFloat;
    DX11EffectFloat m_ContrastFloat;
    DX11EffectFloat m_ShininessFloat;
    DX11EffectFloat m_LightIntensityFloat;
    DX11EffectVector3 m_CameraPositionVector;
    DX11EffectVector3 m_LightDirectionVector;
    DX11EffectVector3 m_AmbientColorVector;
    DX11EffectVector3 m_LightColorVector;
    DX11EffectBool m_UseSpecularBool;
    DX11EffectBool m_UseHalfLambertBool;
    DX11EffectBool m_UseNormalMapBool;
    DX11EffectBool m_FlipGreenChannelBool;
    DX11EffectBool m_RemapNormalRangeBool;
    DX11EffectBool m_UseCookTorranceBool;
    DX11EffectBool m_UseCombustionModulation;
};
