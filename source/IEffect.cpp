#include "pch.h"
#include "IEffect.h"

IEffect::IEffect(ID3D11Device* pDevice, const std::wstring& assetFile, const LPCSTR& techniqueName)
    : m_pEffect{ LoadEffect(pDevice, assetFile) }
{
    assert(m_pEffect->IsValid() && "Effect::Effect() -> Effect not valid!");

    SetTechnique(techniqueName);    
}
IEffect::~IEffect()
{
    SafeRelease(m_pEffect)
}

bool IEffect::SetTechnique(const LPCSTR& techniqueName)
{
    m_pTechnique = m_pEffect->GetTechniqueByName(techniqueName);
    assert(m_pTechnique->IsValid() && "Effect::Effect() -> Technique not valid!");

    return m_pTechnique->IsValid();
}

ID3DX11Effect* IEffect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
    HRESULT result;
    ID3D10Blob* pErrorBlob = nullptr;
    ID3DX11Effect* pEffect = nullptr;
    DWORD shaderFlags = 0;

#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    result = D3DX11CompileEffectFromFile(assetFile.c_str(),
        nullptr,
        nullptr,
        shaderFlags,
        0,
        pDevice,
        &pEffect,
        &pErrorBlob);

    
    if (pErrorBlob)
    {
        const char* pError = static_cast<const char*>(pErrorBlob->GetBufferPointer());
        std::wstringstream ss{};

        for (size_t i = 0; i < pErrorBlob->GetBufferSize(); ++i) ss << pError[i];


        OutputDebugStringW(ss.str().c_str());
        SafeRelease(pErrorBlob);

        std::wcout << ss.str() << '\n';
    }

    if (FAILED(result))
    {
        std::wstringstream ss{};
        ss << "EffectLoader: Failed to load effect!\nPath: " << assetFile;
        std::wcout << ss.str() << '\n';
        return nullptr;
    }

    return pEffect;
}
