#pragma once

#include "Component.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace eng
{
    class Transform : public Component
    {
    public:
        Transform();

        // 局部变换
        void SetPosition(const glm::vec3& pos);
        void SetPosition(float x, float y, float z);
        glm::vec3 GetPosition() const { return m_Position; }

        void SetRotation(const glm::quat& rot) { m_Rotation = rot; m_Dirty = true; }
        glm::quat GetRotation() const { return m_Rotation; }

        void SetScale(const glm::vec3& scale) { m_Scale = scale; m_Dirty = true; }
        void SetScale(float x, float y, float z) { SetScale(glm::vec3(x, y, z)); }
        glm::vec3 GetScale() const { return m_Scale; }

        // 便捷旋转
        void SetEulerAngles(const glm::vec3& euler);
        glm::vec3 GetEulerAngles() const;

        // 世界变换矩阵（自动处理父子层级）
        glm::mat4 GetLocalToWorldMatrix();
        glm::mat4 GetWorldToLocalMatrix();

        // 世界空间便捷属性
        glm::vec3 GetWorldPosition();
        glm::vec3 GetForward() const;
        glm::vec3 GetRight() const;
        glm::vec3 GetUp() const;

        // 父子层级
        void SetParent(Transform* parent);
        Transform* GetParent() const { return m_Parent; }
        const std::vector<Transform*>& GetChildren() const { return m_Children; }
        Transform* GetChild(int index) const;
        int GetChildCount() const;

        const char* GetClassName() const override { return "Transform"; }

    private:
        void MarkDirty();  // 将自己和所有子节点标记为脏
        void RebuildLocalMatrix();

        glm::vec3 m_Position = glm::vec3(0.0f);
        glm::quat m_Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 m_Scale    = glm::vec3(1.0f);

        Transform* m_Parent = nullptr;
        std::vector<Transform*> m_Children;

        bool m_Dirty = true;
        glm::mat4 m_LocalMatrix = glm::mat4(1.0f);
        glm::mat4 m_CachedWorldMatrix = glm::mat4(1.0f);
    };
}