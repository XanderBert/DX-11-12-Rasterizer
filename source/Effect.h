#pragma once

class Effect final
{
public:
    Effect(ID3D11Device* pDevice, const std::wstring& assetFile, const LPCSTR& techniqueName);
    ~Effect();

    Effect(const Effect&) = delete;
    Effect(Effect&&) noexcept = delete;
    Effect& operator=(const Effect&) = delete;
    Effect& operator=(Effect&&) noexcept = delete;


    ID3DX11Effect* GetEffect() const { return m_pEffect; }
    ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }


    void SetWorldViewProjectionMatrix(const dae::Matrix* worldViewProjectionMatrix) const;

private:
    ID3DX11Effect* m_pEffect{ nullptr };
    ID3DX11EffectTechnique* m_pTechnique{ nullptr };
    ID3DX11EffectMatrixVariable* m_pWorldViewProjectionVar{ nullptr };
    
    static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};
