#pragma once
struct SDL_Window;
struct SDL_Surface;
class Camera;

struct Vertex
{
    dae::Vector3 position;
    dae::Vector3 normal;
    dae::Vector3 tangent;
    dae::Vector2 uv;
};


namespace dae
{
    //A render interface
    class IRenderer
    {
    public:
        IRenderer(SDL_Window* pWindow)
        : m_pWindow(pWindow)
        {
            SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
            const Vector3 origin{ 0.f, 0.f, -50.f };
            m_pCamera = std::make_unique<Camera>(origin, 90.f, (static_cast<float>(m_Width) / static_cast<float>(m_Height)));
        }
        virtual ~IRenderer() = default;

        IRenderer(const IRenderer&) = delete;
        IRenderer(IRenderer&&) noexcept = delete;
        IRenderer& operator=(const IRenderer&) = delete;
        IRenderer& operator=(IRenderer&&) noexcept = delete;

        virtual void IncrementFilter() const = 0;

        virtual void Update(const Timer* pTimer)
        {
            m_pCamera->Update(pTimer);
        }

        virtual void OnImGuiRender() = 0;
        virtual void Render() = 0;
        
    protected:
        virtual HRESULT InitializeDirectX() = 0;

        
        //SDL WINDOW
        SDL_Window* m_pWindow{};
        int m_Width{};
        int m_Height{};
        
        //CAMERA
        std::unique_ptr<Camera> m_pCamera;


        
        bool m_IsInitialized{ false };
    private:
    };
}