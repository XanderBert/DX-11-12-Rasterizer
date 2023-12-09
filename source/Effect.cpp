#include "pch.h"
#include "Effect.h"

#include <cassert>

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile,const LPCSTR& techniqueName)
    : m_pEffect{ LoadEffect(pDevice, assetFile) }
{
    assert(m_pEffect->IsValid() && "Effect::Effect() -> Effect not valid!");

    m_pTechnique = m_pEffect->GetTechniqueByName(techniqueName);
    assert(m_pTechnique->IsValid() && "Effect::Effect() -> Technique not valid!");
}

Effect::~Effect()
{
    if(m_pEffect)       m_pEffect->Release();
}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
    HRESULT result;
    ID3D10Blob* pErrorBlob = nullptr;
    ID3DX11Effect* pEffect = nullptr;
    DWORD shaderFlags = 0;
    
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    
    //Effects11: D3DCompileFromFile of fx_5_0 profile failed 80004005: Resources/PosCol3D.fx
    //PosCol3D.fx(1,1): error X3000: Illegal character in shader file
    result = D3DX11CompileEffectFromFile(assetFile.c_str(),
        nullptr,
        nullptr,
        shaderFlags,
        0,
        pDevice,
        &pEffect,
        &pErrorBlob);

    if(FAILED(result))
    {
        std::wstringstream ss;
        
        if(pErrorBlob)
        {
            const char* pError = static_cast<const char*>(pErrorBlob->GetBufferPointer());
            
            for(size_t i = 0; i < pErrorBlob->GetBufferSize(); ++i) ss << pError[i];
                

            OutputDebugStringW(ss.str().c_str());
            pErrorBlob->Release();
            pErrorBlob = nullptr;

            std::wcout << ss.str() << std::endl;
            return nullptr;
        }

        ss << "EffectLoader: Failed to load effect!\nPath: " << assetFile;
        std::wcout << ss.str() << std::endl;
        return nullptr;
    }

    return pEffect;
    
}
