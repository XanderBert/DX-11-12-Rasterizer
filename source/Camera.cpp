#include "pch.h"
#include "Camera.h"

#include "imgui_internal.h"

namespace dae
{
    void Camera::Update(const Timer* pTimer)
    {
        const float deltaTime = pTimer->GetElapsed();

		// Keyboard Input
		const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

		// Mouse Input
		int mouseX{}, mouseY{};
		const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

		// Speed and limit constants
		constexpr float keyboardMovementSpeed{ 10.0f };
		constexpr float mouseMovementSpeed{ 2.0f };
		constexpr float angularSpeed{ 50.0f * TO_RADIANS };

		// The total movement of this frame
		Vector3 direction{};

		// Calculate new position with keyboard inputs
		direction += (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_Z]) * m_Forward * keyboardMovementSpeed * deltaTime;
		direction -= pKeyboardState[SDL_SCANCODE_S] * m_Forward * keyboardMovementSpeed * deltaTime;
		direction -= (pKeyboardState[SDL_SCANCODE_Q] || pKeyboardState[SDL_SCANCODE_A]) * m_Right * keyboardMovementSpeed * deltaTime;
		direction += pKeyboardState[SDL_SCANCODE_D] * m_Right * keyboardMovementSpeed * deltaTime;

#ifndef IMGUI_DISABLE  
    	if(!ImGui::GetIO().WantCaptureMouse)
#endif
		// Calculate new position and rotation with mouse inputs
    	{
    		switch (mouseState)
    		{
    		case SDL_BUTTON_LMASK: // LEFT CLICK
    			direction -= m_Forward * (mouseY * mouseMovementSpeed * deltaTime);
    			m_Yaw += mouseX * angularSpeed * deltaTime;
    			break;
    		case SDL_BUTTON_RMASK: // RIGHT CLICK
    			m_Yaw += mouseX * angularSpeed * deltaTime;
    			m_Pitch -= mouseY * angularSpeed * deltaTime;
    			break;
    		case SDL_BUTTON_X2: // BOTH CLICK
    			direction.y -= mouseY * mouseMovementSpeed * deltaTime;
    			break;
    		}
    	}
		
		m_Pitch = std::clamp(m_Pitch, -89.f * TO_RADIANS, 89.f * TO_RADIANS);


		//Movement calculations
		constexpr float shiftSpeed{ 4.0f };
		direction *= 1.0f + pKeyboardState[SDL_SCANCODE_LSHIFT] * (shiftSpeed - 1.0f);

		m_Origin += direction;
    }

    Matrix Camera::GetViewMatrix() 
    {
    	// // Calculate the new forward vector with the new pitch and yaw
    	const Matrix rotationMatrix = Matrix::CreateRotationX(m_Pitch) * Matrix::CreateRotationY(m_Yaw);
    	m_Forward = rotationMatrix.TransformVector(Vector3::UnitZ).Normalized();
    	m_Right = Vector3::Cross(Vector3::UnitY, m_Forward).Normalized();

    	
    	//Get the inverseViewMatrix
    	const Matrix lookAt = Matrix::CreateLookAtLH(m_Origin, m_Forward, m_Right);

    	//Return the inverse of the inverseViewMatrix
    	return Matrix::Inverse(lookAt);
    }

    Matrix Camera::GetProjectionMatrix() const
    {
        return Matrix::CreatePerspectiveFovLH(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
    }
}
