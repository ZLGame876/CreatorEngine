#include "gameplay/GameplayComponents.h"

#include "CreatorEngine.h"
#include "core/GameObject.h"
#include "core/Scene.h"
#include "graphics/Camera.h"
#include "physics/BoxCollider2D.h"
#include "physics/PhysicsWorld.h"
#include "physics/Rigidbody2D.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

namespace eng
{
    void CharacterController2D::Start()
    {
        m_Rigidbody = GetGameObject() ? GetGameObject()->GetComponent<Rigidbody2D>() : nullptr;
        m_Collider = GetGameObject() ? GetGameObject()->GetComponent<BoxCollider2D>() : nullptr;
        m_SpawnPosition = GetTransform() ? GetTransform()->GetPosition() : glm::vec3(0.0f);
    }

    void CharacterController2D::RefreshGroundedState()
    {
        m_Grounded = false;
        if (!m_Collider || !GetGameObject() || !GetGameObject()->GetScene())
        {
            return;
        }

        const auto& collisions = GetGameObject()->GetScene()->GetPhysicsWorld()->GetCollisions();
        for (const CollisionInfo& collision : collisions)
        {
            if (collision.colliderA == m_Collider)
            {
                // Collision normals point from A to B. A downward normal means B supports A.
                if (collision.normal.y < -0.5f && !collision.colliderB->IsTrigger())
                {
                    m_Grounded = true;
                    return;
                }
            }
            else if (collision.colliderB == m_Collider)
            {
                // When the player is B, an upward A-to-B normal means A is below the player.
                if (collision.normal.y > 0.5f && !collision.colliderA->IsTrigger())
                {
                    m_Grounded = true;
                    return;
                }
            }
        }
    }

    void CharacterController2D::Update(float)
    {
        if (!m_Rigidbody)
        {
            m_Rigidbody = GetGameObject() ? GetGameObject()->GetComponent<Rigidbody2D>() : nullptr;
        }
        RefreshGroundedState();
        if (!m_Rigidbody)
        {
            return;
        }

        InputManager& input = CreatorEngine::GetInstance().GetInputManager();
        const bool left = input.IsKeyPressed(GLFW_KEY_A) || input.IsKeyPressed(GLFW_KEY_LEFT);
        const bool right = input.IsKeyPressed(GLFW_KEY_D) || input.IsKeyPressed(GLFW_KEY_RIGHT);
        const bool jump = input.IsKeyPressed(GLFW_KEY_SPACE);
        const float axis = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);

        glm::vec2 velocity = m_Rigidbody->GetVelocity();
        velocity.x = axis * m_MoveSpeed;
        if (jump && !m_PreviousJumpPressed && m_Grounded)
        {
            velocity.y = m_JumpVelocity;
            m_Grounded = false;
        }
        m_Rigidbody->SetVelocity(velocity);
        m_PreviousJumpPressed = jump;
    }

    void CharacterController2D::Respawn()
    {
        if (GetTransform())
        {
            GetTransform()->SetPosition(m_SpawnPosition);
        }
        if (m_Rigidbody)
        {
            m_Rigidbody->SetVelocity(glm::vec2(0.0f));
            m_Rigidbody->ClearForces();
        }
        m_Grounded = false;
    }

    void CameraFollow2D::Update(float deltaTime)
    {
        Camera* camera = GetGameObject() ? GetGameObject()->GetComponent<Camera>() : nullptr;
        if (!camera || !m_Target || !m_Target->GetTransform() || !GetTransform())
        {
            return;
        }

        const glm::vec3 current = GetTransform()->GetPosition();
        glm::vec3 desired = m_Target->GetTransform()->GetWorldPosition();
        desired.z = current.z;

        const float halfHeight = camera->GetOrthoSize();
        const float halfWidth = halfHeight * (16.0f / 9.0f);
        desired.x = std::clamp(desired.x, m_BoundsMin.x + halfWidth, m_BoundsMax.x - halfWidth);
        desired.y = std::clamp(desired.y, m_BoundsMin.y + halfHeight, m_BoundsMax.y - halfHeight);

        const float blend = 1.0f - std::exp(-std::max(0.0f, m_Smoothing) * std::max(0.0f, deltaTime));
        GetTransform()->SetPosition(glm::mix(current, desired, blend));
    }

    void Patrol2D::Start()
    {
        m_Origin = GetTransform() ? GetTransform()->GetPosition() : glm::vec3(0.0f);
        if (m_Horizontal)
        {
            m_Distance = std::max(0.0f, m_MaxX - m_MinX);
        }
        else
        {
            m_Distance = std::max(0.0f, m_MaxY - m_MinY);
        }
    }

    void Patrol2D::SetHorizontalRange(float minX, float maxX)
    {
        m_Horizontal = true;
        m_MinX = std::min(minX, maxX);
        m_MaxX = std::max(minX, maxX);
        m_Distance = m_MaxX - m_MinX;
    }

    void Patrol2D::SetVerticalRange(float minY, float maxY)
    {
        m_Horizontal = false;
        m_MinY = std::min(minY, maxY);
        m_MaxY = std::max(minY, maxY);
        m_Distance = m_MaxY - m_MinY;
    }

    void Patrol2D::Update(float deltaTime)
    {
        if (!GetTransform() || m_Distance <= 0.0f || m_Speed <= 0.0f)
        {
            return;
        }

        float value = (m_Horizontal ? GetTransform()->GetPosition().x : GetTransform()->GetPosition().y);
        value += m_Direction * m_Speed * std::max(0.0f, deltaTime);
        const float minValue = m_Horizontal ? m_MinX : m_MinY;
        const float maxValue = m_Horizontal ? m_MaxX : m_MaxY;
        if (value >= maxValue)
        {
            value = maxValue;
            m_Direction = -1.0f;
        }
        else if (value <= minValue)
        {
            value = minValue;
            m_Direction = 1.0f;
        }

        glm::vec3 position = GetTransform()->GetPosition();
        if (m_Horizontal) position.x = value;
        else position.y = value;
        GetTransform()->SetPosition(position);
    }

    void HealthComponent::SetMaxHealth(int maxHealth)
    {
        m_MaxHealth = std::max(1, maxHealth);
        m_CurrentHealth = std::min(m_CurrentHealth, m_MaxHealth);
    }

    void HealthComponent::ApplyDamage(int amount)
    {
        if (m_Dead || amount <= 0)
        {
            return;
        }
        m_CurrentHealth = std::max(0, m_CurrentHealth - amount);
        if (m_CurrentHealth == 0)
        {
            m_Dead = true;
            if (m_DeathCallback)
            {
                m_DeathCallback();
            }
        }
    }

    void HealthComponent::Reset()
    {
        m_CurrentHealth = m_MaxHealth;
        m_Dead = false;
    }
}
