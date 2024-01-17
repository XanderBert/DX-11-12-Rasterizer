#include "pch.h"
#include "Mesh.h"

#include <ranges>

#include "PosCol3DEffect.h"
#include "Texture.h"
#include "Utils.h"

Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices,const  std::vector<uint32_t>& indices)
    : m_NumIndices(static_cast<uint32_t>(indices.size()))
{
    //Mapping the texture type to a texture pointer
    //Drawback only 1 texture per type
    //Drawback: Texture instancing is impossible
    m_TextureMap[TextureType::Diffuse] = { nullptr };
    m_TextureMap[TextureType::Normal] = { nullptr };
    m_TextureMap[TextureType::Specular] = { nullptr };
    m_TextureMap[TextureType::Glossiness] = { nullptr };
    m_TextureMap[TextureType::PartialCoverage] = { nullptr };
    
    //There could be 1 instance of the effect class, that will be used over multiple meshes.
    m_pEffect = new TextureEffect(pDevice, L"Resources/TextureShader3D.fx", "DefaultTechnique");
    assert(m_pEffect != nullptr && "Mesh::Mesh() -> Failed to create effect!");

    
    
    // • Create the vertex layout using, again, a matching descriptor.
    static constexpr uint32_t numElements{ 4 };
    D3D11_INPUT_ELEMENT_DESC vertexDescription[numElements]{};

    //Position
    vertexDescription[0].SemanticName = "POSITION";
    vertexDescription[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    vertexDescription[0].AlignedByteOffset = 0;
    vertexDescription[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;


    //Normal
    vertexDescription[1].SemanticName = "NORMAL";
    vertexDescription[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    vertexDescription[1].AlignedByteOffset = sizeof(float) * 3;
    vertexDescription[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    //Tanget
    vertexDescription[2].SemanticName = "TANGENT";
    vertexDescription[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    vertexDescription[2].AlignedByteOffset = sizeof(float) * 6;
    vertexDescription[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    //UV
    vertexDescription[3].SemanticName = "TEXCOORD";
    vertexDescription[3].Format = DXGI_FORMAT_R32G32_FLOAT;
    vertexDescription[3].AlignedByteOffset = sizeof(float) * 9;
    vertexDescription[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    
    // • Through the technique of the effect, create the input layout, using the vertex layout descriptor.
    D3DX11_PASS_DESC passDesc{};
    m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
    const HRESULT result = pDevice->CreateInputLayout(vertexDescription, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout);
    assert(SUCCEEDED(result) && "Mesh::Mesh() -> Failed to create input layout!");
    
    // • Create the DirectX Vertex Buffer and Index Buffer, using the device and a descriptor
    // Vertex Buffer
    D3D11_BUFFER_DESC bufferDescription{};
    bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDescription.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(vertices.size());
    bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDescription.CPUAccessFlags = 0;
    bufferDescription.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresourceData{};
    subresourceData.pSysMem = vertices.data();

    HRESULT hr = pDevice->CreateBuffer(&bufferDescription, &subresourceData, &m_pVertexBuffer);
    assert(SUCCEEDED(hr) && "Mesh::Mesh() -> Failed to create vertex buffer!");

    // Index Buffer
    m_NumIndices = static_cast<uint32_t>(vertices.size());
    bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDescription.ByteWidth = sizeof(uint32_t) * m_NumIndices;
    bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDescription.CPUAccessFlags = 0;
    bufferDescription.MiscFlags = 0;
    subresourceData.pSysMem = indices.data();
    hr = pDevice->CreateBuffer(&bufferDescription, &subresourceData, &m_pIndexBuffer);
    assert(SUCCEEDED(hr) && "Mesh::Mesh() -> Failed to create index buffer!");


    SetupPartialCoverageEffect(pDevice);
}

Mesh::~Mesh()
{
    SafeRelease(m_pVertexBuffer)
    SafeRelease(m_pInputLayout)
    SafeRelease(m_pIndexBuffer)
    SafeDelete(m_pEffect)

    //Delete the textures
    //TODO the mesh does take ownership  of the textures, This makes texture instancing impossible
    for(const Texture* texture : m_TextureMap | std::views::values)
    {
        SafeDelete(texture)
    }

    m_TextureMap.clear();

    
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
{
    
    //Reset the technique
    m_pEffect->SetTechnique("DefaultTechnique");
    
    //------------------------
    // Set primitive topology
    //------------------------
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //------------------------
    // Set input layout
    //------------------------
    pDeviceContext->IASetInputLayout(m_pInputLayout);

    //------------------------
    // Set vertex buffer
    //------------------------
    constexpr UINT stride{ sizeof(Vertex) };
    constexpr UINT offset{0};
    pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

    //------------------------
    // Set index buffer
    //------------------------
    pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);


    //------------------------
    // Draw
    //------------------------
    D3DX11_TECHNIQUE_DESC techDesc{};
    m_pEffect->GetTechnique()->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
        pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
    }

    //Update the Technique
    m_pEffect->SetTechnique("FlatPartialCoverageTechnique");

    //------------------------
    // Set vertex buffer
    //------------------------
    pDeviceContext->IASetVertexBuffers(0, 1, &m_pFireVertexBuffer, &stride, &offset);

    //------------------------
    // Set index buffer
    //------------------------
    pDeviceContext->IASetIndexBuffer(m_pFireIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    //------------------------
    // Draw
    //------------------------
    //TODO: Update the technique
    m_pEffect->GetTechnique()->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
        pDeviceContext->DrawIndexed(m_FireNumIndices, 0, 0);
    }

}

void Mesh::Update(const dae::Timer* pTimer, dae::Matrix* worldProjectionMatrix, dae::Matrix* viewInverseMatrix, dae::Vector3* cameraPosition)
{
    assert(worldProjectionMatrix != nullptr && "Mesh::Update() -> worldViewProjectionMatrix is nullptr!");
    assert(viewInverseMatrix != nullptr && "Mesh::Update() -> viewInverseMatrix is nullptr!");

    
    const dae::Matrix rotationMatrix = dae::Matrix::CreateRotationY((pTimer->GetElapsed() * dae::TO_RADIANS * 5.f));
    m_WorldMatrix *= rotationMatrix;

    const dae::Matrix worldViewPorjectionMatrix = m_WorldMatrix * (*worldProjectionMatrix);

    
    m_pEffect->SetWorldMatrix(&m_WorldMatrix); // Use the combined matrix for the model transformation
    m_pEffect->SetCameraPosition(cameraPosition);
    m_pEffect->SetWorldViewProjectionMatrix(&worldViewPorjectionMatrix);
    m_pEffect->SetViewInverseMatrix(viewInverseMatrix);
}

void Mesh::CreateTexture(const std::string& assetLocation, ID3D11Device* pDevice, Texture*& pTexture)
{
    //Delete the old texture
    delete pTexture;
    pTexture = nullptr;
    
    //Set the new Texture
    pTexture = new Texture(assetLocation, pDevice);
}

void Mesh::SetTextureMap(const std::string& assetLocation, ID3D11Device* pDevice, TextureType type)
{
    Texture*& pTexture = m_TextureMap[type];
    CreateTexture(assetLocation, pDevice, pTexture);
    m_pEffect->SetTextureMap(type, pTexture->GetTextureView());
}

void Mesh::SetupPartialCoverageEffect(ID3D11Device* pDevice)
{
    
    std::vector<Vertex> fireVertices{};
    std::vector<uint32_t> fireIndices{};

    //Load the fire mesh
    dae::Utils::ParseOBJ("Resources/fireFX.obj", fireVertices, fireIndices, false);
    
    // Vertex Buffer
    D3D11_BUFFER_DESC bufferDescription{};
    bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDescription.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(fireVertices.size());
    bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDescription.CPUAccessFlags = 0;
    bufferDescription.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresourceData{};
    subresourceData.pSysMem = fireVertices.data();

    HRESULT hr = pDevice->CreateBuffer(&bufferDescription, &subresourceData, &m_pFireVertexBuffer);
    assert(SUCCEEDED(hr) && "Mesh::Mesh() -> Failed to create Fire vertex buffer!");

    // Index Buffer
    m_FireNumIndices = static_cast<uint32_t>(fireVertices.size());
    bufferDescription.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDescription.ByteWidth = sizeof(uint32_t) * m_FireNumIndices;
    bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDescription.CPUAccessFlags = 0;
    bufferDescription.MiscFlags = 0;
    subresourceData.pSysMem = fireIndices.data();
    hr = pDevice->CreateBuffer(&bufferDescription, &subresourceData, &m_pFireIndexBuffer);
    assert(SUCCEEDED(hr) && "Mesh::Mesh() -> Failed to create Fire index buffer!");
}

