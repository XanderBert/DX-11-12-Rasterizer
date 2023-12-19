#pragma once
#include <map>

#include "TextureEffect.h"

struct Vertex
{
    dae::Vector3 position;
    dae::Vector3 normal;
    dae::Vector3 tangent;
    dae::Vector2 uv;
};

class Texture;
class Effect;
class Mesh final
{
public:
    Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices ,const std::vector<uint32_t>& indices);
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh(Mesh&&) noexcept = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh& operator=(Mesh&&) noexcept = delete;
    
    void Render(ID3D11DeviceContext* pDeviceContext) const;
    void Update(const dae::Timer* pTimer, const dae::Matrix* worldViewProjectionMatrix);
    
    
    
    //TODO: Should be checked if it really behaves properly
    void IncrementFilter(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext);
    void SetTextureMap(const std::string& assetLocation, ID3D11Device* pDevice, TextureType type);

private:
    //Takes a pointer to a pointer to a texture
    static void CreateTexture(const std::string& assetLocation, ID3D11Device* pDevice, Texture*& pTexture);
    std::map<TextureType, Texture*> m_TextureMap;

    
    ID3D11Buffer* m_pVertexBuffer{ nullptr };
    ID3D11InputLayout* m_pInputLayout{ nullptr };
    ID3D11Buffer* m_pIndexBuffer{ nullptr };
    
    TextureEffect* m_pEffect{ nullptr };
    uint32_t m_NumIndices{ 0 };
    D3D11_FILTER m_SamplerFilter{ D3D11_FILTER_MIN_MAG_MIP_LINEAR };
};
