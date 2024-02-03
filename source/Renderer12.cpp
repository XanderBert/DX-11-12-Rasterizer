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

dae::Renderer12::~Renderer12()
{
    //Wait for the command-queue to be empty
    HRESULT hr = S_OK;
    hr = m_pCommandQueue->Signal(m_pFence.Get(), m_FenceValue);
    AssertOnFail(hr)

    hr = m_pFence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
    AssertOnFail(hr)

    if(WaitForSingleObject(m_FenceEvent, 2000) == WAIT_FAILED)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        AssertOnFail(hr)
    }
}

void dae::Renderer12::Update(const Timer* pTimer)
{
    IRenderer::Update(pTimer);
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

    const CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle
    {
    m_pDescriptorHeapRTV->GetCPUDescriptorHandleForHeapStart(),static_cast<INT>(currentBackBufferIndex),
        m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
    };

    
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
        m_pCommandList->ClearRenderTargetView(rtvHandle, clearColorF, 0, nullptr);
    }


    //Set the pipleline state
    {
        m_pCommandList->SetPipelineState(m_pPipelineState.Get());
        m_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());

        //Set IA stage
        m_pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);

        //Set RS
        m_pCommandList->RSSetViewports(1, &m_Viewport);
        m_pCommandList->RSSetScissorRects(1, &m_ScissorRect);

        //bind RenderTarget
        m_pCommandList->OMSetRenderTargets(1, &rtvHandle, TRUE, nullptr);



        const auto worldMatrix =  DirectX::XMMatrixMultiply(m_pCamera->GetViewProjectionMatrixXM(), DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationZ(m_TimeKey)));
        //const auto worldMatrix = (m_pCamera->GetViewProjectionMatrixXM());
        m_pCommandList->SetGraphicsRoot32BitConstants(0, sizeof(DirectX::XMMATRIX) / 4, &worldMatrix, 0);


    	//Draw the triangle
        m_pCommandList->DrawInstanced(m_NrVertices, 1, 0, 0);
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
        hr = m_pCommandQueue->Signal(m_pFence.Get(), ++m_FenceValue);
        AssertOnFail(hr)
    }

    //Present the swapchain
    m_pSwapChain->Present(1, 0);



    //wait for the command list to become free
    //TODO: this is not ideal, right now we wait for the frame to be completed before continuing
    hr  = m_pFence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
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

        // ComPtr<ID3D12Debug1> pDebugController1;
        // hr = pDebugController->QueryInterface(IID_PPV_ARGS(&pDebugController1));
        // ReturnOnFail(hr)
        // pDebugController1->SetEnableGPUBasedValidation(true);
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



//-----------------------------------------------------------------------------------------------------------------------------------------



    //Create Vertex Buffer
    {
        const Vertex vertexData[] =
        {
          {{0.0f, 0.5f, 0.0f }, {1.0f, 0.0f, 0.0f}}, //Top
            {{0.43f, -0.25f, 0.0f},{0.0f, 0.0f, 1.0f} }, //Right
            {{-0.43f, -0.25f, 0.0f},{0.0f, 1.0f, 0.0f} } //Left
        };

        m_NrVertices = _countof(vertexData);

        //Create the resource for the VertexBuffer
        {
            const CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertexData));
            hr = m_pDevice->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_pVertexBuffer)
            );
            ReturnOnFail(hr);
        }

        //Create a resource for the cpu to upload our vertex data to, So that our Gpu can read it and put in in the vertex buffer(which is not accible by the cpu)
        ComPtr<ID3D12Resource> pVertexUploadBuffer;
        {
            const CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertexData));
            hr = m_pDevice->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&pVertexUploadBuffer)
            );
            ReturnOnFail(hr);
        }

        //Copy or Vertex data into the upload Buffer
        {
            Vertex* mappedVertexData = nullptr;
            hr = pVertexUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertexData));
            ReturnOnFail(hr);
            //Copy the data
            std::ranges::copy(vertexData, mappedVertexData);
            pVertexUploadBuffer->Unmap(0, nullptr);
        }


        //Reset Command List and Allocator
        hr = m_pCommandAllocator->Reset();
        ReturnOnFail(hr)
        hr = m_pCommandList->Reset(m_pCommandAllocator.Get(), nullptr);
        ReturnOnFail(hr)

        //Copy Upload buffer into vertex buffer
        m_pCommandList->CopyResource(m_pVertexBuffer.Get(), pVertexUploadBuffer.Get());
        hr = m_pCommandList->Close();
        ReturnOnFail(hr)

        ID3D12CommandList* const commandLists[] = { m_pCommandList.Get() };
        m_pCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

        //Wait for the GPU to be done with the copy before we destroy the upload buffer
        hr = m_pCommandQueue->Signal(m_pFence.Get(), ++m_FenceValue);
        ReturnOnFail(hr)

        hr = m_pFence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
        ReturnOnFail(hr)
        if(WaitForSingleObject(m_FenceEvent, INFINITE) == WAIT_FAILED)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            ReturnOnFail(hr)
        }
    }

    //Create a vertex buffer view
    m_VertexBufferView =
    {
        .BufferLocation = m_pVertexBuffer->GetGPUVirtualAddress(),
        .SizeInBytes = m_NrVertices * static_cast<UINT>(sizeof(Vertex)),
        .StrideInBytes = sizeof(Vertex),
    };


    //Create our root signature
    {
        //Create root signature parameters
        CD3DX12_ROOT_PARAMETER rootParameters[1]{};
        rootParameters[0].InitAsConstants(sizeof(DirectX::XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);


        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init(
            (UINT)std::size(rootParameters),
			rootParameters,
            0,
            nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
        );
        
        //Serialize the root signature
        ComPtr<ID3DBlob> pSerializedRootSignatureBlob;
        ComPtr<ID3DBlob> pErrorBlob;


        if(hr = D3D12SerializeRootSignature(&rootSignatureDescription, D3D_ROOT_SIGNATURE_VERSION_1, &pSerializedRootSignatureBlob, &pErrorBlob); FAILED(hr))
        {
            if(pErrorBlob)
            {
                auto pErrorBuffer = static_cast<const char*>(pErrorBlob->GetBufferPointer());
                LogError("Failed to serialize root signature: " + std::string(pErrorBuffer));
            }
            ReturnOnFail(hr)
        }


        //Create the root signature
        hr = m_pDevice->CreateRootSignature(0, pSerializedRootSignatureBlob->GetBufferPointer(), pSerializedRootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
        ReturnOnFail(hr)
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        struct PipeLineStateStream
        {
            CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE pRootSignature;
            CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT InputLayout;
            CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY PrimitiveTopologyType;
            CD3DX12_PIPELINE_STATE_STREAM_VS VertexShader;
            CD3DX12_PIPELINE_STATE_STREAM_PS PixelShader;
            CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS RTVFormats;
        } pipelineStateStream;
        
        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescriptions[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangent), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Load shaders
        ComPtr<ID3DBlob> pVertexShader;
        hr = D3DReadFileToBlob(L"../bin/Debug/VertexShader.cso", &pVertexShader);
        ReturnOnFail(hr)
        
        ComPtr<ID3DBlob> pPixelShader;
        hr = D3DReadFileToBlob(L"../bin/Debug/PixelShader.cso", &pPixelShader);
        ReturnOnFail(hr)

        //Fill the pipeline state stream (PSO)
        pipelineStateStream.pRootSignature = m_pRootSignature.Get();
        pipelineStateStream.InputLayout = { inputElementDescriptions, _countof(inputElementDescriptions) };
        pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pipelineStateStream.VertexShader = CD3DX12_SHADER_BYTECODE(pVertexShader.Get());
        pipelineStateStream.PixelShader = CD3DX12_SHADER_BYTECODE(pPixelShader.Get());

        D3D12_RT_FORMAT_ARRAY rtvFormats{};
        rtvFormats.NumRenderTargets = 1;
        rtvFormats.RTFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        
        pipelineStateStream.RTVFormats = rtvFormats;

        //Build the actual pipeline state object (PSO)
        const D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc =
        {
            .SizeInBytes = sizeof(PipeLineStateStream),
            .pPipelineStateSubobjectStream = &pipelineStateStream
        };
        
        hr = m_pDevice->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&m_pPipelineState));
        ReturnOnFail(hr)

        //define scissors and viewport
        m_ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
        m_Viewport = CD3DX12_VIEWPORT(0.f, 0.f, static_cast<float>(m_Width), static_cast<float>(m_Height));
    }
    return hr;
}
