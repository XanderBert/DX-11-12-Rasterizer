#pragma once

class Texture final
{
public:
    explicit Texture(const std::string& filename, ID3D11Device* pDevice);
    ~Texture();


    Texture(const Texture& other) = delete;
    Texture(Texture&& other) noexcept = delete;
    Texture& operator=(const Texture& other) = delete;
    Texture& operator=(Texture&& other) noexcept = delete;


    Texture2D* GetTexture() const { return m_pResource; }
    ID3D11ShaderResourceView* GetTextureView() const { return m_pTextureView; }
    
private:
    Texture2D* Load(const std::string& filename, ID3D11Device* pDevice);
    
    Texture2D* m_pResource{ nullptr };
    ID3D11ShaderResourceView* m_pTextureView{ nullptr };
};
