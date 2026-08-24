#include "physics/PhysicsWorld.h"
#include "physics/Rigidbody2D.h"
#include "physics/Collider2D.h"
#include "physics/PhysicsMaterial.h"
#include "core/Transform.h"
#include "core/GameObject.h"
#include <algorithm>
#include <cmath>

namespace eng
{
    PhysicsWorld::PhysicsWorld()
    {
    }

    void PhysicsWorld::Step(float deltaTime)
    {
        // 清空碰撞信息
        m_Collisions.clear();

        // 1. 应用重力
        ApplyGravity(deltaTime);

        // 2. 宽相检测
        BroadPhase();

        // 3. 窄相检测
        NarrowPhase();

        // 4. 碰撞响应（多次迭代）
        for (int i = 0; i < m_VelocityIterations; ++i)
        {
            ResolveCollisions();
        }

        // 5. 位置修正
        for (int i = 0; i < m_PositionIterations; ++i)
        {
            CorrectPositions();
        }

        // 6. 积分更新（速度和位置）
        for (auto* rb : m_Rigidbodies)
        {
            if (rb && !rb->IsKinematic())
            {
                rb->IntegrateVelocity(deltaTime);
                rb->IntegratePosition(deltaTime);
                rb->ClearForces();
            }
        }
    }

    void PhysicsWorld::AddRigidbody(Rigidbody2D* rb)
    {
        if (rb && std::find(m_Rigidbodies.begin(), m_Rigidbodies.end(), rb) == m_Rigidbodies.end())
        {
            m_Rigidbodies.push_back(rb);
        }
    }

    void PhysicsWorld::RemoveRigidbody(Rigidbody2D* rb)
    {
        auto it = std::find(m_Rigidbodies.begin(), m_Rigidbodies.end(), rb);
        if (it != m_Rigidbodies.end())
        {
            m_Rigidbodies.erase(it);
        }
    }

    void PhysicsWorld::AddCollider(Collider2D* collider)
    {
        if (collider && std::find(m_Colliders.begin(), m_Colliders.end(), collider) == m_Colliders.end())
        {
            m_Colliders.push_back(collider);
        }
    }

    void PhysicsWorld::RemoveCollider(Collider2D* collider)
    {
        auto it = std::find(m_Colliders.begin(), m_Colliders.end(), collider);
        if (it != m_Colliders.end())
        {
            m_Colliders.erase(it);
        }
    }

    void PhysicsWorld::ApplyGravity(float deltaTime)
    {
        for (auto* rb : m_Rigidbodies)
        {
            if (!rb || rb->IsKinematic())
                continue;

            // F = m * g * gravityScale
            float gravityScale = rb->GetGravityScale();
            if (gravityScale > 0.0f)
            {
                glm::vec2 gravityForce = m_Gravity * (rb->GetMass() * gravityScale);
                rb->AddForce(gravityForce);
            }
        }
    }

    void PhysicsWorld::BroadPhase()
    {
        // 简单O(n²)宽相检测
        // 实际项目应该使用空间划分（四叉树、网格等）
        for (size_t i = 0; i < m_Colliders.size(); ++i)
        {
            for (size_t j = i + 1; j < m_Colliders.size(); ++j)
            {
                Collider2D* colA = m_Colliders[i];
                Collider2D* colB = m_Colliders[j];

                if (!colA || !colB)
                    continue;

                // AABB重叠测试
                glm::vec2 minA, maxA, minB, maxB;
                colA->GetAABB(minA, maxA);
                colB->GetAABB(minB, maxB);

                bool overlaps = !(maxA.x < minB.x || minA.x > maxB.x ||
                                  maxA.y < minB.y || minA.y > maxB.y);

                if (overlaps)
                {
                    // 加入碰撞候选列表（在NarrowPhase中精确检测）
                    // 这里直接进行窄相检测
                    glm::vec2 normal;
                    float depth;
                    if (colA->TestCollision(colB, normal, depth))
                    {
                        CollisionInfo info;
                        info.colliderA = colA;
                        info.colliderB = colB;
                        info.rigidbodyA = colA->GetAttachedRigidbody();
                        info.rigidbodyB = colB->GetAttachedRigidbody();
                        info.normal = normal;
                        info.depth = depth;

                        // 计算接触点（简化：使用两中心的中点）
                        glm::vec2 centerA = colA->GetWorldCenter();
                        glm::vec2 centerB = colB->GetWorldCenter();
                        info.contactPoint = (centerA + centerB) * 0.5f;

                        m_Collisions.push_back(info);

                        // 触发回调
                        if (m_CollisionCallback)
                        {
                            m_CollisionCallback(info);
                        }
                    }
                }
            }
        }
    }

    void PhysicsWorld::NarrowPhase()
    {
        // 已经在BroadPhase中完成
    }

    void PhysicsWorld::ResolveCollisions()
    {
        for (const auto& collision : m_Collisions)
        {
            // 跳过触发器
            if (collision.colliderA->IsTrigger() || collision.colliderB->IsTrigger())
                continue;

            Rigidbody2D* rbA = collision.rigidbodyA;
            Rigidbody2D* rbB = collision.rigidbodyB;

            // 两个都是静态/运动学刚体，跳过
            if ((!rbA || rbA->IsKinematic()) && (!rbB || rbB->IsKinematic()))
                continue;

            glm::vec2 normal = collision.normal;
            glm::vec2 contactPoint = collision.contactPoint;

            // 计算相对速度
            glm::vec2 velA = rbA ? rbA->GetVelocity() : glm::vec2(0.0f);
            glm::vec2 velB = rbB ? rbB->GetVelocity() : glm::vec2(0.0f);
            glm::vec2 relativeVelocity = velB - velA;

            // 沿法线方向的相对速度
            float velocityAlongNormal = glm::dot(relativeVelocity, normal);

            // 如果物体正在分离，不处理
            if (velocityAlongNormal > 0)
                continue;

            // 计算弹性系数（取两个材质的较小值）
            float restitution = 0.0f;
            PhysicsMaterial* matA = collision.colliderA->GetMaterial();
            PhysicsMaterial* matB = collision.colliderB->GetMaterial();
            if (matA && matB)
            {
                restitution = std::min(matA->GetRestitution(), matB->GetRestitution());
            }
            else if (matA)
            {
                restitution = matA->GetRestitution();
            }
            else if (matB)
            {
                restitution = matB->GetRestitution();
            }

            // 计算冲量大小
            float invMassA = rbA ? rbA->GetInverseMass() : 0.0f;
            float invMassB = rbB ? rbB->GetInverseMass() : 0.0f;
            float invMassSum = invMassA + invMassB;

            if (invMassSum == 0.0f)
                continue;

            // 冲量标量
            float impulseMagnitude = -(1.0f + restitution) * velocityAlongNormal / invMassSum;

            // 应用冲量
            glm::vec2 impulse = impulseMagnitude * normal;

            if (rbA && !rbA->IsKinematic())
            {
                rbA->AddImpulse(-impulse);
            }
            if (rbB && !rbB->IsKinematic())
            {
                rbB->AddImpulse(impulse);
            }

            // 摩擦力处理
            float friction = 0.3f;
            if (matA && matB)
            {
                friction = std::sqrt(matA->GetFriction() * matB->GetFriction());
            }
            else if (matA)
            {
                friction = matA->GetFriction();
            }
            else if (matB)
            {
                friction = matB->GetFriction();
            }

            // 切向冲量（摩擦）
            glm::vec2 tangent = relativeVelocity - velocityAlongNormal * normal;
            float tangentLength = glm::length(tangent);
            if (tangentLength > 0.0001f)
            {
                tangent /= tangentLength;

                float frictionImpulseMagnitude = -glm::dot(relativeVelocity, tangent) / invMassSum;

                // 库仑摩擦定律：|friction impulse| <= friction * |normal impulse|
                float maxFrictionImpulse = friction * std::abs(impulseMagnitude);
                frictionImpulseMagnitude = std::clamp(frictionImpulseMagnitude, -maxFrictionImpulse, maxFrictionImpulse);

                glm::vec2 frictionImpulse = frictionImpulseMagnitude * tangent;

                if (rbA && !rbA->IsKinematic())
                {
                    rbA->AddImpulse(-frictionImpulse);
                }
                if (rbB && !rbB->IsKinematic())
                {
                    rbB->AddImpulse(frictionImpulse);
                }
            }
        }
    }

    void PhysicsWorld::CorrectPositions()
    {
        const float penetrationSlop = 0.01f;  // 允许的最小穿透
        const float correctionPercent = 0.4f;   // 修正百分比

        for (const auto& collision : m_Collisions)
        {
            // 跳过触发器
            if (collision.colliderA->IsTrigger() || collision.colliderB->IsTrigger())
                continue;

            Rigidbody2D* rbA = collision.rigidbodyA;
            Rigidbody2D* rbB = collision.rigidbodyB;

            if ((!rbA || rbA->IsKinematic()) && (!rbB || rbB->IsKinematic()))
                continue;

            float invMassA = rbA ? rbA->GetInverseMass() : 0.0f;
            float invMassB = rbB ? rbB->GetInverseMass() : 0.0f;
            float invMassSum = invMassA + invMassB;

            if (invMassSum == 0.0f)
                continue;

            // 计算修正量
            float correctionMagnitude = std::max(collision.depth - penetrationSlop, 0.0f) / invMassSum * correctionPercent;
            glm::vec2 correction = correctionMagnitude * collision.normal;

            // 应用位置修正
            if (rbA && !rbA->IsKinematic())
            {
                Transform* transformA = rbA->GetTransform();
                if (transformA)
                {
                    glm::vec3 pos = transformA->GetPosition();
                    pos.x -= correction.x * invMassA;
                    pos.y -= correction.y * invMassA;
                    transformA->SetPosition(pos);
                }
            }

            if (rbB && !rbB->IsKinematic())
            {
                Transform* transformB = rbB->GetTransform();
                if (transformB)
                {
                    glm::vec3 pos = transformB->GetPosition();
                    pos.x += correction.x * invMassB;
                    pos.y += correction.y * invMassB;
                    transformB->SetPosition(pos);
                }
            }
        }
    }
}
