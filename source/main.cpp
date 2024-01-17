#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"
#include "Renderer12.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 1280;
	const uint32_t height = 960;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX - Berten Xander 2DAE09",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow) return 1;

	
	//Initialize "framework"
	const auto pTimer = new Timer();

	IRenderer* pRenderer = nullptr;

#if USE_DIRECTX12
	 pRenderer = new Renderer12(pWindow);
#else
	pRenderer = new Renderer(pWindow);
#endif

#ifndef IMGUI_DISABLE  
	//Setup ImGui
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsClassic();
	
	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForD3D(pWindow);
	ImGui_ImplDX11_Init(dynamic_cast<Renderer*>(pRenderer)->GetDevice(), dynamic_cast<Renderer*>(pRenderer)->GetDeviceContext());
#endif
	
	//Start loop
	pTimer->Start();
	//float printTimer = 0.f;
	bool isLooping = true;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
#ifndef IMGUI_DISABLE  
			ImGui_ImplSDL2_ProcessEvent(&e);
#endif
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//Test for a key
				if (e.key.keysym.scancode == SDL_SCANCODE_1) pRenderer->IncrementFilter();
				break;
			default: ;
			}
		}
#ifndef IMGUI_DISABLE  
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Debug");

		pRenderer->OnImGuiRender();
		ImGui::NewLine();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
#endif
		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
#ifndef IMGUI_DISABLE  
		ImGui::Render();
#endif
		pRenderer->Render();
	

		//--------- Timer ---------
		pTimer->Update();

	}
	pTimer->Stop();

	//Shutdown "framework"
#ifndef IMGUI_DISABLE  
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
#endif
		
	delete pRenderer;
	delete pTimer;

	// Cleanup	
	ShutDown(pWindow);
	return 0;
}