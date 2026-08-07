#pragma once

#include "core/Component.h"
#include <glm/glm.hpp>

namespace eng
{
    class PhysicsMaterial;
    class Collider2D;

    // 2D刚体组件：控制物体的物理行为
    class Rigidbody2D : public Component
    {
    public:
        Rigidbody2D();
        ~Rigidbody2D() override;

        // 生命周期
        void Awake() override;
        void Update(float deltaTime) override;

        // 质量属性
        float GetMass() const { return m_Mass; }
        void SetMass(float mass);

        float GetInverseMass() const { return m_InvMass; }

        // 是否运动学刚体（不受力影响，只通过Transform移动）
        bool IsKinematic() const { return m_IsKinematic; }
        void SetKinematic(bool kinematic) { m_IsKinematic = kinematic; }

        // 速度
        const glm::vec2& GetVelocity() const { return m_Velocity; }
        void SetVelocity(const glm::vec2& velocity) { m_Velocity = velocity; }

        // 角速度（弧度/秒）
        float GetAngularVelocity() const { return m_AngularVelocity; }
        void SetAngularVelocity(float angularVel) { m_AngularVelocity = angularVel; }

        // 力
        void AddForce(const glm::vec2& force);
        void AddImpulse(const glm::vec2& impulse);

        // 扭矩
        void AddTorque(float torque);
        void AddAngularImpulse(float impulse);

        // 阻尼
        float GetLinearDamping() const { return m_LinearDamping; }
        void SetLinearDamping(float damping) { m_LinearDamping = damping; }

        float GetAngularDamping() const { return m_AngularDamping; }
        void SetAngularDamping(float damping) { m_AngularDamping = damping; }

        // 重力缩放
        float GetGravityScale() const { return m_GravityScale; }
        void SetGravityScale(float scale) { m_GravityScale = scale; }

        // 物理材质
        void SetMaterial(PhysicsMaterial* material) { m_Material = material; }
        PhysicsMaterial* GetMaterial() const { return m_Material; }

        // 内部使用：积分更新
        void IntegrateVelocity(float deltaTime);
        void IntegratePosition(float deltaTime);

        // 清除累积的力
        void ClearForces();

        // 序列化
        nlohmann::json Serialize() const override;
        void Deserialize(const nlohmann::json& json) override;

        const char* GetClassName() const override { return "Rigidbody2D"; }

    private:
        float m_Mass = 1.0f;
        float m_InvMass = 1.0f;  // 1/mass，质量为0时为0

        bool m_IsKinematic = false;

        glm::vec2 m_Velocity = glm::vec2(0.0f);
        float m_AngularVelocity = 0.0f;

        glm::vec2 m_Force = glm::vec2(0.0f);
        float m_Torque = 0.0f;

        float m_LinearDamping = 0.0f;
        float m_AngularDamping = 0.05f;

        float m_GravityScale = 1.0f;

        PhysicsMaterial* m_Material = nullptr;
    };
}
