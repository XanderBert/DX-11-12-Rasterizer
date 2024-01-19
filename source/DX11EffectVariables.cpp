#include "pch.h"
#include "Dx11EffectVariables.h"


//float
DX11EffectBool::DX11EffectBool(ID3DX11Effect* pEffect, const std::string& variableName, bool value)
{
    pVariable = pEffect->GetVariableByName(variableName.c_str())->AsScalar();
    if (!pVariable->IsValid())
    {
        const std::string error = "Effect variable not found: " + variableName;
        throw std::runtime_error(error);
    }
    
    m_Value = value;
    UpdateEffect();
}

void DX11EffectBool::Toggle()
{
    m_Value = !m_Value;
    UpdateEffect();
}

void DX11EffectBool::Set(bool value)
{
    m_Value = value;
    UpdateEffect();
}

bool DX11EffectBool::Get()
{
    pVariable->GetBool(&m_Value);
    return m_Value;
}

bool* DX11EffectBool::GetAddressOf()
{
    return &m_Value;
}

void DX11EffectBool::UpdateEffect() const
{
    pVariable->SetBool(m_Value);
}





//Vector3
DX11EffectVector3::DX11EffectVector3(ID3DX11Effect* pEffect, const std::string& variableName, const dae::Vector3& value)
{
    pVariable = pEffect->GetVariableByName(variableName.c_str())->AsVector();

    if (!pVariable->IsValid())
    {
        const std::string error = "Effect variable not found: " + variableName;
        throw std::runtime_error(error);
    }
    
    m_Value[0] = value.x;
    m_Value[1] = value.y;
    m_Value[2] = value.z;
    
    UpdateEffect();
}

void DX11EffectVector3::Set(const dae::Vector3& value)
{
    m_Value[0] = value.x;
    m_Value[1] = value.y;
    m_Value[2] = value.z;

    UpdateEffect();
}

dae::Vector3 DX11EffectVector3::Get()
{
    pVariable->GetFloatVector(m_Value);
    return dae::Vector3{ m_Value[0], m_Value[1], m_Value[2] };
}

float* DX11EffectVector3::GetAddressOf()
{
    return m_Value;
}

void DX11EffectVector3::UpdateEffect() const
{
    pVariable->SetFloatVector(m_Value);
}








//float
DX11EffectFloat::DX11EffectFloat(ID3DX11Effect* pEffect, const std::string& variableName, float value)
{
    pVariable = pEffect->GetVariableByName(variableName.c_str())->AsScalar();

    if (!pVariable->IsValid())
    {
        const std::string error = "Effect variable not found: " + variableName;
        throw std::runtime_error(error);
    }
    
    m_Value = value;
    
    UpdateEffect();
}

void DX11EffectFloat::Set(float value)
{
    m_Value = value;
    UpdateEffect();
}

float DX11EffectFloat::Get()
{
    pVariable->GetFloat(&m_Value);
    return m_Value;
}

float* DX11EffectFloat::GetAddressOf()
{
    return &m_Value;
}

void DX11EffectFloat::UpdateEffect() const
{
    pVariable->SetFloat(m_Value);
}
