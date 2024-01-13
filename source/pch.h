#pragma once

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





//Windows Headers
#include <wrl.h>



// Framework Headers
#include "Timer.h"
#include "Math.h"
#include "Camera.h"
#include "Macros.h"



//Hr result macro
#define ReturnOnFail(hr) if(FAILED(hr)) return hr;
#define ReturnAndAssertOnFail(hr) if(FAILED(hr)) { assert(false && "HRESULT failed!"); return; }