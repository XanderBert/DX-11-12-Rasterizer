# DX11/12-Rasterizer

Basic Rasterizer that uses DirectX11 and DirectX12.

**Additions:**

* A working DirecX12 Render Loop.
* CookTorrence Lighting Calculations with the Nayar Reflectance Model
* Option for Blinn/Phong
* Option for Half Lambert
* A debug menu in ImGui to change shader parameters on the flight.
* Added a waving contrast for the FireFX and making it less predictable by using waves in waves.
* In DirectX 11 Setup
  * Using an updated factory
  * Using DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL backbuffer.
  * Added a system for reading/writing  for some effect variables.
    * Bool
    * Float3
    * FLoat

The goal of this project is to get a understanding the differences between DirectX11 and DirectX12

Also to better understand how the renderpipeline works.

![DX11_12.gif](assets/DX11_12.gif)
