#pragma once

struct SDL_Window;
struct SDL_Surface;
class Camera;

namespace dae
{
    class Renderer12 final
    {
    public:
        Renderer12(SDL_Window* pWindow);
        ~Renderer12();

        Renderer12(const Renderer12&) = delete;
        Renderer12(Renderer12&&) noexcept = delete;
        Renderer12& operator=(const Renderer12&) = delete;
        Renderer12& operator=(Renderer12&&) noexcept = delete;

        void Update(const Timer* pTimer);
        void Render();


        void IncrementFilter() const;

    private:
        SDL_Window* m_pWindow{};
        UINT m_Width{};
        UINT m_Height{};
        
        //CAMERA
        std::unique_ptr<Camera> m_pCamera;
		
        //DIRECTX
        bool m_IsInitialized{ false };
        HRESULT InitializeDirectX();

        
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
