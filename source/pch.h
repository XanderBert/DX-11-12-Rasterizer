#pragma once
#define WIN32_LEAN_AND_MEAN
#define USE_DIRECTX12 1

//i currently don't have ImGui working with DirectX12
#if USE_DIRECTX12
    #define IMGUI_DISABLE
#endif


#include <iostream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>
#include <cassert> 
#define NOMINMAX  //for directx

// SDL Headers
#include "SDL.h"
#include "SDL_syswm.h"
#include "SDL_surface.h"
#include "SDL_image.h"

// DirectX12 Headers
#include <d3dx12.h>
#include <dxgi1_5.h>

// DirectX11 Headers
#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

//Custom Wraper for effect variables
#include "Dx11EffectVariables.h"



//Windows Headers
#include <wrl.h>


// Framework Headers
#include "Timer.h"
#include "Math.h"
#include "Camera.h"
#include "Macros.h"


//ImGui

#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_dx11.h"



typedef ID3D11Texture2D Texture2D;