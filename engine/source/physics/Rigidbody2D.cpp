#include "physics/Rigidbody2D.h"
#include "physics/PhysicsMaterial.h"
#include "physics/PhysicsWorld.h"
#include "core/Transform.h"
#include "core/GameObject.h"
#include "core/Scene.h"
#include <algorithm>
#include <cmath>

namespace eng
{
    Rigidbody2D::Rigidbody2D()
    {
    }

    Rigidbody2D::~Rigidbody2D()
    {
        // 从 PhysicsWorld 注销
        GameObject* go = GetGameObject();
        if (go)
        {
            Scene* scene = go->GetScene();
            if (scene && scene->GetPhysicsWorld())
            {
                scene->GetPhysicsWorld()->RemoveRigidbody(this);
            }
        }
    }

    void Rigidbody2D::Awake()
    {
        // 确保初始质量设置正确
        SetMass(m_Mass);

        // 注册到 PhysicsWorld
        GameObject* go = GetGameObject();
        if (go)
        {
            Scene* scene = go->GetScene();
            if (scene && scene->GetPhysicsWorld())
            {
                scene->GetPhysicsWorld()->AddRigidbody(this);
            }
        }
    }

    void Rigidbody2D::Update(float deltaTime)
    {
        // Rigidbody2D 的更新由 PhysicsWorld 驱动
        // 这里只清除累积的力（如果 PhysicsWorld 没有调用 Update）
        // 注意：PhysicsWorld::Step 会调用 IntegrateVelocity 和 IntegratePosition
    }

    void Rigidbody2D::SetMass(float mass)
    {
        m_Mass = std::max(0.0f, mass);
        m_InvMass = (m_Mass > 0.0f) ? (1.0f / m_Mass) : 0.0f;
    }

    void Rigidbody2D::AddForce(const glm::vec2& force)
    {
        if (m_IsKinematic || m_InvMass == 0.0f)
            return;
        m_Force += force;
    }

    void Rigidbody2D::AddImpulse(const glm::vec2& impulse)
    {
        if (m_IsKinematic || m_InvMass == 0.0f)
            return;
        // 冲量直接改变速度：Δv = impulse / mass
        m_Velocity += impulse * m_InvMass;
    }

    void Rigidbody2D::AddTorque(float torque)
    {
        if (m_IsKinematic)
            return;
        m_Torque += torque;
    }

    void Rigidbody2D::AddAngularImpulse(float impulse)
    {
        if (m_IsKinematic)
            return;
        // 简化处理：假设转动惯量为质量的函数
        // 实际应该由Collider计算
        float invInertia = (m_InvMass > 0.0f) ? (1.0f / (m_Mass * 10.0f)) : 0.0f;
        m_AngularVelocity += impulse * invInertia;
    }

    void Rigidbody2D::IntegrateVelocity(float deltaTime)
    {
        if (m_IsKinematic || m_InvMass == 0.0f)
            return;

        // 应用力：a = F/m
        glm::vec2 acceleration = m_Force * m_InvMass;

        // 更新速度：v += a * dt
        m_Velocity += acceleration * deltaTime;

        // 应用线性阻尼
        if (m_LinearDamping > 0.0f)
        {
            m_Velocity *= std::max(0.0f, 1.0f - m_LinearDamping * deltaTime);
        }

        // 应用扭矩：α = τ/I
        float invInertia = (m_InvMass > 0.0f) ? (1.0f / (m_Mass * 10.0f)) : 0.0f;
        float angularAcceleration = m_Torque * invInertia;

        // 更新角速度：ω += α * dt
        m_AngularVelocity += angularAcceleration * deltaTime;

        // 应用角阻尼
        if (m_AngularDamping > 0.0f)
        {
            m_AngularVelocity *= std::max(0.0f, 1.0f - m_AngularDamping * deltaTime);
        }
    }

    void Rigidbody2D::IntegratePosition(float deltaTime)
    {
        if (m_IsKinematic)
            return;

        Transform* transform = GetTransform();
        if (!transform)
            return;

        // 更新位置
        glm::vec3 pos = transform->GetPosition();
        pos.x += m_Velocity.x * deltaTime;
        pos.y += m_Velocity.y * deltaTime;
        transform->SetPosition(pos);

        // 更新旋转（绕Z轴）
        if (std::abs(m_AngularVelocity) > 0.0001f)
        {
            float angle = m_AngularVelocity * deltaTime;
            glm::vec3 euler = transform->GetEulerAngles();
            euler.z += glm::degrees(angle);
            transform->SetEulerAngles(euler);
        }
    }

    void Rigidbody2D::ClearForces()
    {
        m_Force = glm::vec2(0.0f);
        m_Torque = 0.0f;
    }

    nlohmann::json Rigidbody2D::Serialize() const
    {
        nlohmann::json json;
        json["type"] = "Rigidbody2D";
        json["mass"] = m_Mass;
        json["kinematic"] = m_IsKinematic;
        json["velocity"] = { m_Velocity.x, m_Velocity.y };
        json["angularVelocity"] = m_AngularVelocity;
        json["linearDamping"] = m_LinearDamping;
        json["angularDamping"] = m_AngularDamping;
        json["gravityScale"] = m_GravityScale;
        return json;
    }

    void Rigidbody2D::Deserialize(const nlohmann::json& json)
    {
        if (json.contains("mass"))
            SetMass(json["mass"].get<float>());
        if (json.contains("kinematic"))
            m_IsKinematic = json["kinematic"].get<bool>();
        if (json.contains("velocity"))
        {
            auto v = json["velocity"];
            m_Velocity = glm::vec2(v[0].get<float>(), v[1].get<float>());
        }
        if (json.contains("angularVelocity"))
            m_AngularVelocity = json["angularVelocity"].get<float>();
        if (json.contains("linearDamping"))
            m_LinearDamping = json["linearDamping"].get<float>();
        if (json.contains("angularDamping"))
            m_AngularDamping = json["angularDamping"].get<float>();
        if (json.contains("gravityScale"))
            m_GravityScale = json["gravityScale"].get<float>();
    }
}
