#pragma once

#include "physics/Collider2D.h"

namespace eng
{
    // 2D盒型碰撞体
    class BoxCollider2D : public Collider2D
    {
    public:
        BoxCollider2D();
        ~BoxCollider2D() override = default;

        // 尺寸（宽高）
        const glm::vec2& GetSize() const { return m_Size; }
        void SetSize(const glm::vec2& size) { m_Size = size; }
        void SetSize(float width, float height) { m_Size = glm::vec2(width, height); }

        // 获取AABB包围盒
        void GetAABB(glm::vec2& min, glm::vec2& max) const override;

        // 碰撞检测
        bool TestCollision(const Collider2D* other, glm::vec2& normal, float& depth) const override;

        // 序列化
        nlohmann::json Serialize() const override;
        void Deserialize(const nlohmann::json& json) override;

        const char* GetClassName() const override { return "BoxCollider2D"; }

    private:
        glm::vec2 m_Size = glm::vec2(1.0f);
    };
}
