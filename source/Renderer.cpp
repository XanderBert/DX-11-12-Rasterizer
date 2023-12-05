#include "pch.h"
#include "Renderer.h"

#include <cassert>

#include "Mesh.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

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

	Renderer::~Renderer()
	{
		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		if (m_pRenderTargetBuffer) m_pRenderTargetBuffer->Release();
		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pDepthStencilBuffer) m_pDepthStencilBuffer->Release();
		if (m_pSwapChain) m_pSwapChain->Release();
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
		if (m_pDevice) m_pDevice->Release();
		
	}

	void Renderer::Update(const Timer* pTimer)
	{

	}


	void Renderer::Render() const
	{
		assert(m_IsInitialized == true && "DirectX is not initialized!");
		if (!m_IsInitialized) return;
		
		//Clear the RenderTargetView and the DepthStencilView
		constexpr ColorRGB clearColor = ColorRGB{ 0.39f, 0.59f, 0.93f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);



		const std::vector<Vertex> vertices
		{
			Vertex{ 0.f, 0.5f, 0.f, 1.f, 0.f, 0.f },
			Vertex{ 0.5f, -0.5f, 0.f, 0.f, 1.f, 0.f },
			Vertex{ -0.5f, -0.5f, 0.f, 0.f, 0.f, 1.f }
		};
		const std::vector<uint32_t> indices
		{
			0, 1, 2
		};

		const Mesh triangle{ m_pDevice, vertices, indices };
		triangle.Render(m_pDeviceContext);
		//Swap
		m_pSwapChain->Present(0, 0);

	}

	HRESULT Renderer::InitializeDirectX()
	{
		constexpr D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		//
		//Create the Device and the Device Context
		//
		HRESULT result = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			createDeviceFlags,
			&featureLevel,
			1,
			D3D11_SDK_VERSION,
			&m_pDevice,
			nullptr,
			&m_pDeviceContext);
		
		if(FAILED(result)) return result;


		//
		//Create DXGI Factory1
		//
		IDXGIFactory1* pFactory{};
		//result = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pFactory));
		if(FAILED(result)) return result;


		//
		//Create SwapChain
		//
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1; 
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;								// 60Hz
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						// 32 bits, 8 bits per channel
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// Scanline order is not important
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					// No scaling
		swapChainDesc.SampleDesc.Count = 1;													// No anti-aliasing
		swapChainDesc.SampleDesc.Quality = 0;												// No anti-aliasing
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;						// Use the buffer as a render target
		swapChainDesc.BufferCount = 1;														// Use a single buffer
		swapChainDesc.Windowed = true;														//Windowed
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;								// Discard the previous buffer when swapping
		swapChainDesc.Flags = 0;															// No flags
		
		//Get the handle for the backbuffer
		SDL_SysWMinfo wmInfo;
		SDL_GetVersion(&wmInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &wmInfo);
		swapChainDesc.OutputWindow = wmInfo.info.win.window;								//set the output window to the SDL_Backbuffer

		//Create the actual swapchain
		result = pFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if(FAILED(result)) return result;

		//
		//Create the DepthStencil and the DepthStencilView
		//
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;													//No mipmapping
		depthStencilDesc.ArraySize = 1;													//No texture array
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;						//24 bits for depth, 8 bits for stencil
		depthStencilDesc.SampleDesc.Count = 1;											//No anti-aliasing
		depthStencilDesc.SampleDesc.Quality = 0;										//No anti-aliasing
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;									//Standard usage
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;							//Bind as depth stencil
		depthStencilDesc.CPUAccessFlags = 0;											//No CPU access
		depthStencilDesc.MiscFlags = 0;													//No misc flags


		//Create the view
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;							//Same format as the depth stencil
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;				//Same dimension as the depth stencil
		depthStencilViewDesc.Texture2D.MipSlice = 0;									//MipSlice 0

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if(FAILED(result)) return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if(FAILED(result)) return result;


		//
		//Create RnedertargetView and RenderTarget
		//
		//result = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&m_pRenderTargetBuffer));
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if(FAILED(result)) return result;

		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if(FAILED(result)) return result;

		
		//
		//Bind the views to the output merger stage
		//
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);


		//
		//Set the viewport
		//
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
		m_pDeviceContext->RSSetViewports(1, &viewport);
		
		return S_OK;
	}
}


