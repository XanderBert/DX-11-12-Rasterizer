#pragma once
#include <map>

#include "IRenderer.h"
#include "Matrix.h"
#include "TextureEffect.h"


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
    void Update(const dae::Timer* pTimer,const dae::Matrix& viewProjectionMatrix, const dae::Vector3& cameraPosition);
    
    
    void SetTextureMap(const std::string& assetLocation, ID3D11Device* pDevice, TextureType type);
    void SetupPartialCoverageEffect(ID3D11Device* pDevice);

    TextureEffect* GetEffect() const
    {
        assert(m_pEffect != nullptr && "Mesh::GetEffect() -> m_pEffect is nullptr!");
        return m_pEffect;
    }
    
    void ToggleRotation() { m_RotationEnabled = !m_RotationEnabled; }
    void SetRotation(bool enabled) { m_RotationEnabled = enabled; }
    bool* GetpRotation() { return &m_RotationEnabled; }

    void ToggleFireEffect() { m_UseFireEffect = !m_UseFireEffect; }
    void SetFireEffect(bool enabled) { m_UseFireEffect = enabled; }
    bool* GetpFireEffect() { return &m_UseFireEffect; }
    
private:
    //Takes a pointer to a pointer to a texture
    static void CreateTexture(const std::string& assetLocation, ID3D11Device* pDevice, Texture*& pTexture);
    std::map<TextureType, Texture*> m_TextureMap;

    
    ID3D11InputLayout* m_pInputLayout{ nullptr };
    ID3D11Buffer* m_pVertexBuffer{ nullptr };
    ID3D11Buffer* m_pIndexBuffer{ nullptr };
    uint32_t m_NumIndices{ 0 };

    ID3D11Buffer* m_pFireVertexBuffer{ nullptr };
    ID3D11Buffer* m_pFireIndexBuffer{ nullptr };
    uint32_t m_FireNumIndices{ 0 };
    
    TextureEffect* m_pEffect{ nullptr };
    
    
    dae::Matrix m_WorldMatrix{dae::Matrix::Identity()};

    
    bool m_RotationEnabled{ true };
    bool m_UseFireEffect{ true };
};
