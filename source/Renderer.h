#pragma once

struct SDL_Window;
struct SDL_Surface;
class Mesh;
class Camera;

namespace dae
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

	private:
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		//MESH
		void InitializeMesh();
		std::unique_ptr<Mesh> m_pMesh;

		//CAMERA
		std::unique_ptr<Camera> m_pCamera;
		
		//DIRECTX
		bool m_IsInitialized{ false };
		HRESULT InitializeDirectX();
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGISwapChain1* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;
		
	};
}
