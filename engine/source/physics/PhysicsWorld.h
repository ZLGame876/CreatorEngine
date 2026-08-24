#pragma once

#include "core/Object.h"
#include <glm/glm.hpp>
#include <vector>
#include <functional>

namespace eng
{
    class Rigidbody2D;
    class Collider2D;
    class Scene;

    // 碰撞信息
    struct CollisionInfo
    {
        Collider2D* colliderA = nullptr;
        Collider2D* colliderB = nullptr;
        Rigidbody2D* rigidbodyA = nullptr;
        Rigidbody2D* rigidbodyB = nullptr;
        glm::vec2 normal = glm::vec2(0.0f);
        float depth = 0.0f;
        glm::vec2 contactPoint = glm::vec2(0.0f);
    };

    // 2D物理世界：管理物理模拟
    class PhysicsWorld : public Object
    {
    public:
        PhysicsWorld();
        ~PhysicsWorld() override = default;

        // 重力
        const glm::vec2& GetGravity() const { return m_Gravity; }
        void SetGravity(const glm::vec2& gravity) { m_Gravity = gravity; }

        // 模拟步进
        void Step(float deltaTime);

        // 注册/注销刚体和碰撞体
        void AddRigidbody(Rigidbody2D* rb);
        void RemoveRigidbody(Rigidbody2D* rb);
        void AddCollider(Collider2D* collider);
        void RemoveCollider(Collider2D* collider);

        // 碰撞回调
        using CollisionCallback = std::function<void(const CollisionInfo&)>;
        void SetCollisionCallback(CollisionCallback callback) { m_CollisionCallback = callback; }

        // 获取所有碰撞对（调试用）
        const std::vector<CollisionInfo>& GetCollisions() const { return m_Collisions; }

        // 迭代次数（用于碰撞求解器）
        int GetVelocityIterations() const { return m_VelocityIterations; }
        void SetVelocityIterations(int iterations) { m_VelocityIterations = iterations; }

        int GetPositionIterations() const { return m_PositionIterations; }
        void SetPositionIterations(int iterations) { m_PositionIterations = iterations; }

        const char* GetClassName() const override { return "PhysicsWorld"; }

    private:
        // 宽相检测：AABB重叠测试
        void BroadPhase();

        // 窄相检测：精确碰撞检测
        void NarrowPhase();

        // 碰撞响应：应用冲量
        void ResolveCollisions();

        // 位置修正：防止穿透
        void CorrectPositions();

        // 应用重力
        void ApplyGravity(float deltaTime);

        glm::vec2 m_Gravity = glm::vec2(0.0f, -9.81f);

        std::vector<Rigidbody2D*> m_Rigidbodies;
        std::vector<Collider2D*> m_Colliders;

        std::vector<CollisionInfo> m_Collisions;

        int m_VelocityIterations = 8;
        int m_PositionIterations = 3;

        CollisionCallback m_CollisionCallback;
    };
}
