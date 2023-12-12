#pragma once

struct Vertex
{
    float x, y, z;
    float r, g, b;
};

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

private:
    ID3D11Buffer* m_pVertexBuffer{ nullptr };
    ID3D11InputLayout* m_pInputLayout{ nullptr };
    ID3D11Buffer* m_pIndexBuffer{ nullptr };
    
    Effect* m_pEffect{ nullptr };

    
    uint32_t m_NumIndices{ 0 };

    
};
