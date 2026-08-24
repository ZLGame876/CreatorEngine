#include "graphics/Camera.h"
#include "core/Transform.h"

namespace eng
{
    Camera* Camera::s_MainCamera = nullptr;

    void Camera::Awake()
    {
        if (!s_MainCamera)
        {
            s_MainCamera = this;
        }
    }

    void Camera::OnDestroy()
    {
        if (s_MainCamera == this)
        {
            s_MainCamera = nullptr;
        }
    }

    void Camera::SetOrthographic(float size, float nearPlane, float farPlane)
    {
        m_ProjectionType = ProjectionType::Orthographic;
        m_OrthoSize = size;
        m_NearPlane = nearPlane;
        m_FarPlane = farPlane;
    }

    void Camera::SetPerspective(float fov, float nearPlane, float farPlane)
    {
        m_ProjectionType = ProjectionType::Perspective;
        m_FOV = fov;
        m_NearPlane = nearPlane;
        m_FarPlane = farPlane;
    }

    glm::mat4 Camera::GetViewMatrix() const
    {
        Transform* transform = GetTransform();
        if (!transform) return glm::mat4(1.0f);

        glm::vec3 pos = transform->GetWorldPosition();
        glm::vec3 forward = transform->GetForward();
        glm::vec3 up = transform->GetUp();

        return glm::lookAt(pos, pos + forward, up);
    }

    glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const
    {
        if (m_ProjectionType == ProjectionType::Orthographic)
        {
            float halfH = m_OrthoSize;
            float halfW = halfH * aspectRatio;
            return glm::ortho(-halfW, halfW, -halfH, halfH, m_NearPlane, m_FarPlane);
        }
        else
        {
            return glm::perspective(glm::radians(m_FOV), aspectRatio, m_NearPlane, m_FarPlane);
        }
    }

    glm::mat4 Camera::GetViewProjectionMatrix(float aspectRatio) const
    {
        return GetProjectionMatrix(aspectRatio) * GetViewMatrix();
    }

    Camera* Camera::GetMain()
    {
        return s_MainCamera;
    }

    nlohmann::json Camera::Serialize() const
    {
        nlohmann::json data;
        data["projectionType"] = static_cast<int>(m_ProjectionType);
        data["orthoSize"] = m_OrthoSize;
        data["fov"] = m_FOV;
        data["nearPlane"] = m_NearPlane;
        data["farPlane"] = m_FarPlane;
        return data;
    }

    void Camera::Deserialize(const nlohmann::json& json)
    {
        if (json.contains("projectionType"))
        {
            int pt = json["projectionType"].get<int>();
            if (pt == 0)
                SetOrthographic(
                    json.value("orthoSize", 5.0f),
                    json.value("nearPlane", -10.0f),
                    json.value("farPlane", 10.0f)
                );
            else
                SetPerspective(
                    json.value("fov", 60.0f),
                    json.value("nearPlane", 0.1f),
                    json.value("farPlane", 100.0f)
                );
        }
    }
}
