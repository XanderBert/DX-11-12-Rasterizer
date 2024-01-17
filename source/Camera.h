#pragma once
namespace dae
{
    class Camera final
    {
    public:
        Camera(const Vector3& origin, float fov, float aspectRatio) :
            m_Fov(fov * (TO_RADIANS * 0.5f)),
            m_AspectRatio(aspectRatio),
            m_Origin(origin)
        {}

        Camera(const Vector3& origin, float fov, float aspectRatio, float nearPlane, float farPlane) :
            m_Fov(fov * (TO_RADIANS * 0.5f)),
            m_AspectRatio(aspectRatio),
            m_Origin(origin),
            m_NearPlane(nearPlane),
            m_FarPlane(farPlane)
        {}
    
        ~Camera() = default;

        Camera(const Camera&) = delete;
        Camera& operator=(const Camera&) = delete;
        Camera(Camera&&) = delete;
        Camera& operator=(Camera&&) = delete;

        void Update(const Timer* pTimer);

        
        Matrix GetViewMatrix();
        Matrix GetProjectionMatrix() const;
        Matrix GetViewInverseMatrix() { return Matrix::Inverse(GetViewMatrix()); }

        Vector3 GetPosition() const { return m_Origin; }
    
        void SetFOV(float fov) { m_Fov = fov * (TO_RADIANS * 0.5f); }
        float GetFOV() const { return m_Fov; }
    
        void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; }
        float GetAspectRatio() const { return m_AspectRatio; }
    
    private:
        float m_Fov = 100.f;
        float m_AspectRatio = 16.f / 9.f;
        Vector3 m_Origin = { 0.f, 0.f, 0.f };

        float m_NearPlane = 0.1f;
        float m_FarPlane = 1000.f;


        Vector3 m_Forward = { 0.f, 0.f, 1.f };
        Vector3 m_Right = { 1.f, 0.f, 0.f };

        float m_Pitch = 0.f;
        float m_Yaw = 0.f;
    
    };
}