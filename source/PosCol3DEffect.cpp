#include "pch.h"
#include "PosCol3DEffect.h"

PosCol3DEffect::PosCol3DEffect(ID3D11Device* pDevice, const std::wstring& assetFile, const LPCSTR& techniqueName)
    : IEffect(pDevice, assetFile, techniqueName)
{
    //Get the WorldViewProjection variable from the shader and store it in a private variable
    m_pWorldViewProjectionVar = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
    assert(m_pWorldViewProjectionVar->IsValid() && "Effect::Effect() -> WorldViewProjection variable not valid!");
}


void PosCol3DEffect::SetWorldViewProjectionMatrix(const dae::Matrix* worldViewProjectionMatrix) const
{
    //reinterpret to pointer to float
    m_pWorldViewProjectionVar->SetMatrix(reinterpret_cast<const float*>(worldViewProjectionMatrix));
}