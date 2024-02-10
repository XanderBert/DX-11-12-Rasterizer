#pragma once
#include "IRenderer.h"
#include "DirectXMath.h"
struct SDL_Window;
struct SDL_Surface;
class Camera;



namespace dae
{
    class Renderer12 final : public IRenderer
    {
    public:
        Renderer12(SDL_Window* pWindow);
        virtual ~Renderer12() override;
        
        virtual void Update(const Timer* pTimer) override;
        virtual void OnImGuiRender() override;
        virtual void Render() override;
        
        virtual void IncrementFilter() const override;

    private:
        HRESULT InitializeDirectX() override;
        
        //DirectX 12 Variables
        const static UINT m_BufferCount{ 2 };
        Microsoft::WRL::ComPtr<ID3D12Device2> m_pDevice;
        Microsoft::WRL::ComPtr<IDXGISwapChain4> m_pSwapChain;
        Microsoft::WRL::ComPtr<ID3D12Resource> pRenderTargetBackBuffers[m_BufferCount];
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pCommandAllocator;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDescriptorHeapRTV;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_pCommandQueue;

        Microsoft::WRL::ComPtr<ID3D12Fence> m_pFence;
        UINT64 m_FenceValue{};
        HANDLE m_FenceEvent{};



        //Testing
        float m_TimeKey{};
        const float m_TimeKeyIncrement{ 0.01f };

        //For Mesh / Effect classes
        
        std::vector<Vertex> m_Vertices;
        UINT m_NrVertices{};

    	std::vector<UINT> m_Indices;
        UINT m_NrIndices{};


        Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pPipelineState;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRootSignature;

        Microsoft::WRL::ComPtr<ID3D12Resource> m_pVertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView{};

    	Microsoft::WRL::ComPtr<ID3D12Resource> m_pIndexBuffer;
        D3D12_INDEX_BUFFER_VIEW m_IndexBufferView{};

        CD3DX12_RECT m_ScissorRect{};
        CD3DX12_VIEWPORT m_Viewport{};

        Microsoft::WRL::ComPtr<ID3D12Resource> m_pDepthBuffer;
        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pDepthStencilViewHeap;

        //Microsoft::WRL::ComPtr<D3D12_CPU_DESCRIPTOR_HANDLE> m_pDepthStencilViewHandle;

        //For Mesh class
        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void CreateRootSignature();
        void CreatePipelineState();
        void CreateDepthBuffer();
    };
}
