#pragma once

#include "core/Component.h"
#include <glm/glm.hpp>
#include <functional>
#include <utility>

namespace eng
{
    class BoxCollider2D;
    class Rigidbody2D;
    class GameObject;

    // A small, reusable 2D platform-game controller built on the engine physics API.
    class CharacterController2D : public Component
    {
    public:
        CharacterController2D() = default;

        void Start() override;
        void Update(float deltaTime) override;

        void SetMoveSpeed(float speed) { m_MoveSpeed = speed; }
        float GetMoveSpeed() const { return m_MoveSpeed; }
        void SetJumpVelocity(float velocity) { m_JumpVelocity = velocity; }
        float GetJumpVelocity() const { return m_JumpVelocity; }

        bool IsGrounded() const { return m_Grounded; }
        void Respawn();
        const glm::vec3& GetSpawnPosition() const { return m_SpawnPosition; }
        Rigidbody2D* GetRigidbody() const { return m_Rigidbody; }
        BoxCollider2D* GetCollider() const { return m_Collider; }

        const char* GetClassName() const override { return "CharacterController2D"; }

    private:
        void RefreshGroundedState();

        Rigidbody2D* m_Rigidbody = nullptr;
        BoxCollider2D* m_Collider = nullptr;
        glm::vec3 m_SpawnPosition = glm::vec3(0.0f);
        float m_MoveSpeed = 280.0f;
        float m_JumpVelocity = 620.0f;
        bool m_Grounded = false;
        bool m_PreviousJumpPressed = false;
    };

    // Keeps a camera centered on a target while respecting world bounds.
    class CameraFollow2D : public Component
    {
    public:
        CameraFollow2D() = default;

        void SetTarget(GameObject* target) { m_Target = target; }
        GameObject* GetTarget() const { return m_Target; }
        void SetBounds(const glm::vec2& min, const glm::vec2& max)
        {
            m_BoundsMin = min;
            m_BoundsMax = max;
        }
        void SetSmoothing(float smoothing) { m_Smoothing = smoothing; }

        void Update(float deltaTime) override;

        const char* GetClassName() const override { return "CameraFollow2D"; }

    private:
        GameObject* m_Target = nullptr;
        glm::vec2 m_BoundsMin = glm::vec2(-100000.0f);
        glm::vec2 m_BoundsMax = glm::vec2(100000.0f);
        float m_Smoothing = 12.0f;
    };

    // Ping-pong motion for moving platforms, lifts, or enemies.
    class Patrol2D : public Component
    {
    public:
        Patrol2D() = default;

        void Start() override;
        void Update(float deltaTime) override;

        void SetHorizontalRange(float minX, float maxX);
        void SetVerticalRange(float minY, float maxY);
        void SetSpeed(float speed) { m_Speed = speed; }

        const char* GetClassName() const override { return "Patrol2D"; }

    private:
        glm::vec3 m_Origin = glm::vec3(0.0f);
        float m_MinX = 0.0f;
        float m_MaxX = 0.0f;
        float m_MinY = 0.0f;
        float m_MaxY = 0.0f;
        float m_Speed = 100.0f;
        float m_Distance = 0.0f;
        float m_Direction = 1.0f;
        bool m_Horizontal = true;
    };

    class HealthComponent : public Component
    {
    public:
        using DeathCallback = std::function<void()>;

        explicit HealthComponent(int maxHealth = 1) : m_MaxHealth(maxHealth), m_CurrentHealth(maxHealth) {}

        void SetMaxHealth(int maxHealth);
        int GetMaxHealth() const { return m_MaxHealth; }
        int GetCurrentHealth() const { return m_CurrentHealth; }
        bool IsDead() const { return m_Dead; }
        void SetDeathCallback(DeathCallback callback) { m_DeathCallback = std::move(callback); }
        void ApplyDamage(int amount);
        void Reset();

        const char* GetClassName() const override { return "HealthComponent"; }

    private:
        int m_MaxHealth = 1;
        int m_CurrentHealth = 1;
        bool m_Dead = false;
        DeathCallback m_DeathCallback;
    };

    // Marker components make gameplay code independent of object names.
    class Hazard2D : public Component
    {
    public:
        explicit Hazard2D(int damage = 1) : m_Damage(damage) {}
        int GetDamage() const { return m_Damage; }
        void SetDamage(int damage) { m_Damage = damage; }
        const char* GetClassName() const override { return "Hazard2D"; }
    private:
        int m_Damage = 1;
    };

    class Goal2D : public Component
    {
    public:
        const char* GetClassName() const override { return "Goal2D"; }
    };
}
