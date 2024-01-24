#include "pch.h"
#include "Renderer12.h"

#include <cmath>
#include <numbers>

#include "Utils.h"

using Microsoft::WRL::ComPtr;

dae::Renderer12::Renderer12(SDL_Window* pWindow) :IRenderer(pWindow)
{
    //Initialize DirectX pipeline
    if (InitializeDirectX() == S_OK)
    {
        m_IsInitialized = true;
        std::cout << "DirectX is initialized and ready!\n";
    }
    else
    {
        std::cout << "DirectX initialization failed!\n";
    }
}

void dae::Renderer12::Update(const Timer* /*pTimer*/)
{
}

void dae::Renderer12::OnImGuiRender()
{
#ifndef IMGUI_DISABLE
    
#endif
}

void dae::Renderer12::Render()
{
    assert(m_IsInitialized && "DirectX is not initialized!");
    if(!m_IsInitialized) return;

    HRESULT hr = S_OK;
    
    hr  = m_pCommandAllocator->Reset();
    AssertOnFail(hr)
    
    
    hr = m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);
    AssertOnFail(hr)

    const UINT currentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
    const auto& currentBackBuffer = pRenderTargetBackBuffers[currentBackBufferIndex];
    
    //Clear the render target
    //transition the render target from the present state to the render target state so the command list draws to it starting from here
    {        
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            currentBackBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );

        m_pCommandList->ResourceBarrier(1, &barrier);

        //Clear the buffer
        //constexpr ColorRGB clearColor = ColorRGB{ 0.39f, 0.59f, 0.93f };
        const ColorRGB clearColor = ColorRGB
        {
            sin(2.f * m_TimeKey + 1.f) * 0.5f + 0.5f,
            sin(3.f * m_TimeKey + 2.f) * 0.5f + 0.5f,
            sin(4.f * m_TimeKey + 3.f) * 0.5f + 0.5f
        };
        const float clearColorF[4] = { clearColor.r, clearColor.g, clearColor.b, 1.f };

        const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle
        {
            m_pDescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart(),
            static_cast<INT>(currentBackBufferIndex),
            m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
        };

        m_pCommandList->ClearRenderTargetView(rtvHandle, clearColorF, 0, nullptr);
    }


    //Transition from render target state to present state
    {
        const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition
        (
            currentBackBuffer.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
        );

        m_pCommandList->ResourceBarrier(1, &barrier);
    }

    
    

    //transition the render target from the render target state to the present state so the swap chain can present it
    {
        hr =  m_pCommandList->Close();
        AssertOnFail(hr)

        ID3D12CommandList* ppCommandLists[] = { m_pCommandList.Get() };
        m_pCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
    }


    //insert a fence to mark the command list completion
    {
        hr = m_pCommandQueue->Signal(m_pFence.Get(), m_FenceValue);
        AssertOnFail(hr)
        ++m_FenceValue;
    }

    //Present the swapchain
    m_pSwapChain->Present(1, 0);



    //wait for the command list to become free
    //TODO: this is not ideal, right now we wait for the frame to be completed before continuing
    hr  = m_pFence->SetEventOnCompletion(m_FenceValue - 1, m_FenceEvent);
    AssertOnFail(hr)
    if(WaitForSingleObject(m_FenceEvent, INFINITE) == WAIT_FAILED)
    {
        assert(false && "WaitForSingleObject failed!");
    }


    if((m_TimeKey += m_TimeKeyIncrement) >= 2.f * std::numbers::pi_v<float>)
    {
        m_TimeKey = 0.f;
    }

}

void dae::Renderer12::IncrementFilter() const
{
}

HRESULT dae::Renderer12::InitializeDirectX()
{
    HRESULT hr = S_OK;

    //Enable DX12 Debug Layer if needed
#ifdef _DEBUG
    {
        ComPtr<ID3D12Debug> pDebugController;
        hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebugController));
        ReturnOnFail(hr)
        pDebugController->EnableDebugLayer();
    }
#endif

    
    //Create a factory
    ComPtr<IDXGIFactory4> pFactory;
    
    hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&pFactory));
    ReturnOnFail(hr)

    //Create a device
    hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDevice));
    ReturnOnFail(hr)

    //Command queue
    //The CPU sends commands on a command list, The GPU then executes these commands
    {
        constexpr D3D12_COMMAND_QUEUE_DESC desc =
        {
            //A Direct Que can be used for any operation
            //A Compute Que can be used for compute and copy operations
            //A Copy Que can only be used for copy operations
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0,
        };

        hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pCommandQueue));
        ReturnOnFail(hr)
    }


    //Swapchain
    {
        //Create Swapchain Descriptor
        const DXGI_SWAP_CHAIN_DESC1 swapChainDesc =
        {
            .Width = static_cast<UINT>(m_Width),
            .Height = static_cast<UINT>(m_Height),
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .Stereo = FALSE,
            .SampleDesc = {.Count = 1, .Quality = 0},
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = m_BufferCount,
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
            .Flags = 0,
        };

        // Get the handle for the backbuffer using SDL
        SDL_SysWMinfo wmInfo;
        SDL_GetVersion(&wmInfo.version);
        SDL_GetWindowWMInfo(m_pWindow, &wmInfo);
        const HWND WHNDL = wmInfo.info.win.window;


        
        ComPtr<IDXGISwapChain1> pSwapChain1;
        hr = pFactory->CreateSwapChainForHwnd(
            m_pCommandQueue.Get(),
            WHNDL,
            &swapChainDesc,
            nullptr,
            nullptr,
            &pSwapChain1
        );
        ReturnOnFail(hr)

        //'upgrade' to IDXGISwapChain4
        pSwapChain1.As(&m_pSwapChain);
    }


    //Create Descriptor Heap for RenderTargetViews
    {
        constexpr D3D12_DESCRIPTOR_HEAP_DESC desc =
        {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .NumDescriptors = m_BufferCount,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = 0,
        };

        hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pDescriptorHeapRTV));
        ReturnOnFail(hr)
    }

    const auto rtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


    //buffer references
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_pDescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart());
        
        for (UINT i{}; i < m_BufferCount; ++i)
        {
            //Store the back buffer reference
            hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pRenderTargetBackBuffers[i]));
            ReturnOnFail(hr)

            //Create a render target view for the back buffer
            m_pDevice->CreateRenderTargetView(pRenderTargetBackBuffers[i].Get(), nullptr, rtvHandle);

            //Move the handle to the next descriptor
            rtvHandle.Offset(1, rtvDescriptorSize);
        }
    }



    //Create the Command Allocator
    hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator));
    ReturnOnFail(hr)

    //Create the Command List
    hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList));
    ReturnOnFail(hr)

    //Close the command list so it can be reset at the start of the next frame
    m_pCommandList->Close();



    //fence -> used to synchronize the CPU and GPU
    //We can set signal values and put them in the queue, when that value is reached we know what resources are available to use.
    //We can wait for a given value or we can ask the current value

    {
        hr = m_pDevice->CreateFence(m_FenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_pFence));
        ReturnOnFail(hr)
    }
    
    //create an event handle to use for frame synchronization
    m_FenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!m_FenceEvent)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        ReturnOnFail(hr)
    }

    return hr;
}
