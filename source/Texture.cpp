#include "pch.h"
#include "Texture.h"

Texture::Texture(const std::string& filename, ID3D11Device* pDevice)
{
    ID3D11Texture2D* pTexture = Load(filename, pDevice);

    if(pTexture == nullptr)
    {
        std::cout << "Texture::Texture() failed to load texture: " << filename << std::endl;\
        assert(false && "Texture::Texture() failed to load texture!");
    }
}

Texture::~Texture()
{
    SafeRelease(m_pResource)
    SafeRelease(m_pTextureView)
}

ID3D11Texture2D* Texture::Load(const std::string& filename, ID3D11Device* pDevice)
{
    //Load the image
    SDL_Surface* pSurface{ IMG_Load(filename.c_str())};

    //Check if the image loaded
    assert(pSurface != nullptr && "Texture::Load() failed to load a surface!");
    if(!pSurface)
    {
        SDL_FreeSurface(pSurface);
        return nullptr;
    }

    //Setup the DXGI_FORMAT
    DXGI_FORMAT dxFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };         //32 bit color -> 8 bits per channel

    //Setup the Texture Description
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = pSurface->w;
    desc.Height = pSurface->h;
    desc.MipLevels = 1;                                         
    desc.ArraySize = 1;
    desc.Format = dxFormat;
    
    desc.SampleDesc.Count = 1;                                  //no aliasing
    desc.SampleDesc.Quality = 0;

    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;                //We are going to use it as a shader texture
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;



    //Create the SubResourceData that holds the pointer to the image data
    D3D11_SUBRESOURCE_DATA initData{};
    initData.pSysMem = pSurface->pixels;
    initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
    initData.SysMemSlicePitch = static_cast<UINT>(pSurface->pitch * pSurface->h);

    //Create the Texture
    HRESULT result{pDevice->CreateTexture2D(&desc, &initData, &m_pResource)};

    //Check if the texture was created successfully
    assert(result == S_OK && "Texture::Load() failed to create texture!");
    if (result != S_OK)
    {
        SDL_FreeSurface(pSurface);
        return nullptr;
    }


    //Create the Texture View Description
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = dxFormat;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;

    //Create the Texture View
    result = pDevice->CreateShaderResourceView(m_pResource, &srvDesc, &m_pTextureView);

    //free the surface as its no longer needed at this point
    SDL_FreeSurface(pSurface);

    
    //Check if the texture view was created successfully
    assert(result == S_OK && "Texture::Load() failed to create texture view!");
    if (result != S_OK) return nullptr;

    return m_pResource;
}
