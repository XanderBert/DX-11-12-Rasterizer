#pragma once

class IEffect
{
public:
    IEffect(ID3D11Device* pDevice, const std::wstring& assetFile, const LPCSTR& techniqueName);
    virtual ~IEffect();


    IEffect(const IEffect&) = delete;
    IEffect(IEffect&&) noexcept = delete;
    IEffect& operator=(const IEffect&) = delete;
    IEffect& operator=(IEffect&&) noexcept = delete;

    bool SetTechnique(const LPCSTR& techniqueName);
    
    ID3DX11Effect* GetEffect() const { return m_pEffect; }
    ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }

protected:
    ID3DX11Effect* m_pEffect{ nullptr };
    ID3DX11EffectTechnique* m_pTechnique{ nullptr };
private:
    static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};
