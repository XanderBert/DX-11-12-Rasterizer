#pragma once
#include "Vector3.h"

class DX11EffectBool final
{

public:
    DX11EffectBool(ID3DX11Effect* pEffect, const std::string& variableName, bool value = false);
    ~DX11EffectBool() = default;

    DX11EffectBool(const DX11EffectBool& other) = delete;
    DX11EffectBool(DX11EffectBool&& other) noexcept = delete;
    DX11EffectBool& operator=(const DX11EffectBool& other) = delete;
    DX11EffectBool& operator=(DX11EffectBool&& other) noexcept = delete;
    
    void Toggle();
    void Set(bool value);
    bool Get();
    bool* GetAddressOf();
    
private:
    void UpdateEffect() const;
    ID3DX11EffectScalarVariable* pVariable;
    bool m_Value;
};


class DX11EffectVector3 final
{
public:
    DX11EffectVector3(ID3DX11Effect* pEffect, const std::string& variableName, const dae::Vector3&  value = dae::Vector3{0,0,0});
    ~DX11EffectVector3() = default;

    DX11EffectVector3(const DX11EffectVector3& other) = delete;
    DX11EffectVector3(DX11EffectVector3&& other) noexcept = delete;
    DX11EffectVector3& operator=(const DX11EffectVector3& other) = delete;
    DX11EffectVector3& operator=(DX11EffectVector3&& other) noexcept = delete;
    

    void Set(const dae::Vector3& value);
    dae::Vector3 Get();
    float* GetAddressOf();
    void UpdateEffect() const;

private:
    ID3DX11EffectVectorVariable* pVariable;
    float m_Value[3];
};



class DX11EffectFloat final
{
public:
    DX11EffectFloat(ID3DX11Effect* pEffect, const std::string& variableName, float value = 0.f);
    ~DX11EffectFloat() = default;

    DX11EffectFloat(const DX11EffectFloat& other) = delete;
    DX11EffectFloat(DX11EffectFloat&& other) noexcept = delete;
    DX11EffectFloat& operator=(const DX11EffectFloat& other) = delete;
    DX11EffectFloat& operator=(DX11EffectFloat&& other) noexcept = delete;
    

    void Set(float value);
    float Get();
    float* GetAddressOf();
    void UpdateEffect() const;

private:
    ID3DX11EffectScalarVariable* pVariable;
    float m_Value;
};