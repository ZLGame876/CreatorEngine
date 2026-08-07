#include "core/Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <algorithm>

namespace eng
{
    Transform::Transform()
    {
    }

    void Transform::SetPosition(const glm::vec3& pos)
    {
        m_Position = pos;
        MarkDirty();
    }

    void Transform::SetPosition(float x, float y, float z)
    {
        SetPosition(glm::vec3(x, y, z));
    }

    void Transform::SetEulerAngles(const glm::vec3& euler)
    {
        m_Rotation = glm::quat(glm::radians(euler));
        MarkDirty();
    }

    glm::vec3 Transform::GetEulerAngles() const
    {
        return glm::degrees(glm::eulerAngles(m_Rotation));
    }

    void Transform::RebuildLocalMatrix()
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), m_Position);
        glm::mat4 R = glm::mat4_cast(m_Rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), m_Scale);
        m_LocalMatrix = T * R * S;
        m_Dirty = false;
    }

    glm::mat4 Transform::GetLocalToWorldMatrix()
    {
        if (m_Dirty)
        {
            RebuildLocalMatrix();
        }

        if (m_Parent)
        {
            m_CachedWorldMatrix = m_Parent->GetLocalToWorldMatrix() * m_LocalMatrix;
        }
        else
        {
            m_CachedWorldMatrix = m_LocalMatrix;
        }
        return m_CachedWorldMatrix;
    }

    glm::mat4 Transform::GetWorldToLocalMatrix()
    {
        return glm::inverse(GetLocalToWorldMatrix());
    }

    glm::vec3 Transform::GetWorldPosition()
    {
        return glm::vec3(GetLocalToWorldMatrix()[3]);
    }

    glm::vec3 Transform::GetForward() const
    {
        return m_Rotation * glm::vec3(0.0f, 0.0f, -1.0f);
    }

    glm::vec3 Transform::GetRight() const
    {
        return m_Rotation * glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 Transform::GetUp() const
    {
        return m_Rotation * glm::vec3(0.0f, 1.0f, 0.0f);
    }

    void Transform::SetParent(Transform* parent)
    {
        // 从旧父节点移除
        if (m_Parent)
        {
            auto& siblings = m_Parent->m_Children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        }

        m_Parent = parent;

        if (m_Parent)
        {
            m_Parent->m_Children.push_back(this);
        }

        MarkDirty();
    }

    Transform* Transform::GetChild(int index) const
    {
        if (index >= 0 && index < static_cast<int>(m_Children.size()))
        {
            return m_Children[index];
        }
        return nullptr;
    }

    int Transform::GetChildCount() const
    {
        return static_cast<int>(m_Children.size());
    }

    void Transform::MarkDirty()
    {
        m_Dirty = true;
        for (Transform* child : m_Children)
        {
            child->MarkDirty();
        }
    }
}