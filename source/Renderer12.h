#pragma once
#include "IRenderer.h"

struct SDL_Window;
struct SDL_Surface;
class Camera;

namespace dae
{
    class Renderer12 final : public IRenderer
    {
    public:
        Renderer12(SDL_Window* pWindow);
        virtual ~Renderer12() override = default;
        
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
    };
}
