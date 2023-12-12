#include "pch.h"
#include "Mesh.h"

#include <cassert>
#include "Effect.h"

Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices,const  std::vector<uint32_t>& indices)
    : m_NumIndices(static_cast<uint32_t>(indices.size()))
{
    //There could be 1 instance of the effect class, that will be used over multiple meshes.
   m_pEffect = new Effect(pDevice, L"Resources/PosCol3D.fx", "DefaultTechnique");

    
    // • Create the vertex layout using, again, a matching descriptor.
    static constexpr uint32_t numElements{ 2 };
    D3D11_INPUT_ELEMENT_DESC vertexDescription[numElements]{};

    vertexDescription[0].SemanticName = "POSITION";
    vertexDescription[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    vertexDescription[0].AlignedByteOffset = 0;
    vertexDescription[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    
    vertexDescription[1].SemanticName = "COLOR";
    vertexDescription[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    vertexDescription[1].AlignedByteOffset = 12;
    vertexDescription[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    
    
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
}

Mesh::~Mesh()
{
    if (m_pVertexBuffer) m_pVertexBuffer->Release();
    if (m_pInputLayout) m_pInputLayout->Release();
    if (m_pIndexBuffer) m_pIndexBuffer->Release();
    delete m_pEffect;
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext) const
{
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
}

void Mesh::Update(const dae::Timer* pTimer, const dae::Matrix* worldViewProjectionMatrix)
{
    assert(worldViewProjectionMatrix != nullptr && "Mesh::Update() -> worldViewProjectionMatrix is nullptr!");
    m_pEffect->SetWorldViewProjectionMatrix(worldViewProjectionMatrix);
}
