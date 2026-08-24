#pragma once

#include "core/Object.h"

namespace eng
{
    // 物理材质：定义碰撞表面的摩擦和弹性属性
    class PhysicsMaterial : public Object
    {
    public:
        PhysicsMaterial(float friction = 0.4f, float restitution = 0.0f);
        ~PhysicsMaterial() override = default;

        // 摩擦系数 (0 = 无摩擦, 1 = 高摩擦)
        float GetFriction() const { return m_Friction; }
        void SetFriction(float friction) { m_Friction = friction; }

        // 弹性系数 (0 = 完全非弹性, 1 = 完全弹性)
        float GetRestitution() const { return m_Restitution; }
        void SetRestitution(float restitution) { m_Restitution = restitution; }

        const char* GetClassName() const override { return "PhysicsMaterial"; }

    private:
        float m_Friction;
        float m_Restitution;
    };
}
