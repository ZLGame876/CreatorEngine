#include "core/Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <algorithm>

namespace eng
{
    Transform::Transform()
    {
    }

    Transform::~Transform()
    {
        while (!m_Children.empty())
        {
            m_Children.back()->SetParent(nullptr, true);
        }
        SetParent(nullptr, false);
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

    void Transform::SetRotation(const glm::quat& rot)
    {
        m_Rotation = glm::normalize(rot);
        MarkDirty();
    }

    void Transform::SetScale(const glm::vec3& scale)
    {
        m_Scale = scale;
        MarkDirty();
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

    void Transform::RebuildLocalMatrix() const
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), m_Position);
        glm::mat4 R = glm::mat4_cast(m_Rotation);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), m_Scale);
        m_LocalMatrix = T * R * S;
        m_Dirty = false;
    }

    glm::mat4 Transform::GetLocalToWorldMatrix() const
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

    glm::mat4 Transform::GetWorldToLocalMatrix() const
    {
        return glm::inverse(GetLocalToWorldMatrix());
    }

    glm::vec3 Transform::GetWorldPosition() const
    {
        return glm::vec3(GetLocalToWorldMatrix()[3]);
    }

    glm::vec3 Transform::GetForward() const
    {
        const glm::mat4 world = GetLocalToWorldMatrix();
        return glm::normalize(-glm::vec3(world[2]));
    }

    glm::vec3 Transform::GetRight() const
    {
        const glm::mat4 world = GetLocalToWorldMatrix();
        return glm::normalize(glm::vec3(world[0]));
    }

    glm::vec3 Transform::GetUp() const
    {
        const glm::mat4 world = GetLocalToWorldMatrix();
        return glm::normalize(glm::vec3(world[1]));
    }

    bool Transform::SetParent(Transform* parent, bool worldPositionStays)
    {
        if (parent == m_Parent)
        {
            return true;
        }

        if (parent == this)
        {
            return false;
        }

        for (Transform* ancestor = parent; ancestor; ancestor = ancestor->m_Parent)
        {
            if (ancestor == this)
            {
                return false;
            }
        }

        glm::mat4 worldMatrix(1.0f);
        if (worldPositionStays)
        {
            worldMatrix = GetLocalToWorldMatrix();
        }

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

        if (worldPositionStays)
        {
            const glm::mat4 localMatrix = m_Parent
                ? glm::inverse(m_Parent->GetLocalToWorldMatrix()) * worldMatrix
                : worldMatrix;

            glm::vec3 skew;
            glm::vec4 perspective;
            glm::quat rotation;
            glm::vec3 position;
            glm::vec3 scale;
            if (glm::decompose(localMatrix, scale, rotation, position, skew, perspective))
            {
                m_Position = position;
                m_Rotation = glm::normalize(rotation);
                m_Scale = scale;
            }
        }

        MarkDirty();
        return true;
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
