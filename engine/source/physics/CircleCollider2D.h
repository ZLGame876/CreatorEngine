#pragma once

#include "physics/Collider2D.h"

namespace eng
{
    // 2D圆形碰撞体
    class CircleCollider2D : public Collider2D
    {
    public:
        CircleCollider2D();
        ~CircleCollider2D() override = default;

        // 半径
        float GetRadius() const { return m_Radius; }
        void SetRadius(float radius) { m_Radius = radius; }

        // 获取AABB包围盒
        void GetAABB(glm::vec2& min, glm::vec2& max) const override;

        // 碰撞检测
        bool TestCollision(const Collider2D* other, glm::vec2& normal, float& depth) const override;

        // 序列化
        nlohmann::json Serialize() const override;
        void Deserialize(const nlohmann::json& json) override;

        const char* GetClassName() const override { return "CircleCollider2D"; }

    private:
        float m_Radius = 0.5f;
    };
}
