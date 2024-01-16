#pragma once
#include "IRenderer.h"
class Mesh;

namespace dae
{
	class Renderer final : public IRenderer
	{
	public:
		Renderer(SDL_Window* pWindow);
		virtual ~Renderer() override;
				
		virtual void Update(const Timer* pTimer) override;
		virtual void Render() override;
		
		virtual void IncrementFilter() const override;

	private:
		virtual HRESULT InitializeDirectX() override;
		
		//MESH
		void InitializeMesh();
		std::unique_ptr<Mesh> m_pMesh;

		
		//DIRECTX11 Specific
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain1* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;
	};
}
