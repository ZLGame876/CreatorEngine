#pragma once

#include "core/Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <nlohmann/json.hpp>

namespace eng
{
    class Camera : public Component
    {
    public:
        enum class ProjectionType
        {
            Orthographic,
            Perspective
        };

        Camera() = default;

        // 投影设置
        void SetOrthographic(float size, float nearPlane = -10.0f, float farPlane = 10.0f);
        void SetPerspective(float fov, float nearPlane = 0.1f, float farPlane = 100.0f);

        ProjectionType GetProjectionType() const { return m_ProjectionType; }

        float GetOrthoSize() const { return m_OrthoSize; }
        float GetFOV() const { return m_FOV; }
        float GetNearPlane() const { return m_NearPlane; }
        float GetFarPlane() const { return m_FarPlane; }

        // 矩阵计算
        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix(float aspectRatio) const;
        glm::mat4 GetViewProjectionMatrix(float aspectRatio) const;

        // 序列化
        nlohmann::json Serialize() const override;
        void Deserialize(const nlohmann::json& json) override;

        void Awake() override;
        void OnDestroy() override;

        const char* GetClassName() const override { return "Camera"; }

        // 静态快捷方法：获取主相机
        static Camera* GetMain();

    private:
        ProjectionType m_ProjectionType = ProjectionType::Orthographic;
        float m_OrthoSize = 5.0f;
        float m_FOV = 60.0f;
        float m_NearPlane = -10.0f;
        float m_FarPlane = 10.0f;

        static Camera* s_MainCamera;
    };
}
