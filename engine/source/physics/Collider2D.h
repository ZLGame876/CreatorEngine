#pragma once

#include "core/Component.h"
#include <glm/glm.hpp>

namespace eng
{
    class PhysicsMaterial;
    class Rigidbody2D;

    // 碰撞体基类
    class Collider2D : public Component
    {
    public:
        Collider2D();
        ~Collider2D() override;

        void Awake() override;

        // 偏移量（相对于Transform中心）
        const glm::vec2& GetOffset() const { return m_Offset; }
        void SetOffset(const glm::vec2& offset) { m_Offset = offset; }

        // 是否为触发器（只检测碰撞，不产生物理响应）
        bool IsTrigger() const { return m_IsTrigger; }
        void SetTrigger(bool trigger) { m_IsTrigger = trigger; }

        // 物理材质
        PhysicsMaterial* GetMaterial() const { return m_Material; }
        void SetMaterial(PhysicsMaterial* material) { m_Material = material; }

        // 获取所属刚体（可能为空）
        Rigidbody2D* GetAttachedRigidbody() const;

        // 获取世界空间中的中心位置
        glm::vec2 GetWorldCenter() const;

        // 获取AABB包围盒（用于宽相检测）
        virtual void GetAABB(glm::vec2& min, glm::vec2& max) const = 0;

        // 碰撞检测方法（由PhysicsWorld调用）
        virtual bool TestCollision(const Collider2D* other, glm::vec2& normal, float& depth) const = 0;

        // 序列化
        nlohmann::json Serialize() const override;
        void Deserialize(const nlohmann::json& json) override;

        const char* GetClassName() const override { return "Collider2D"; }

    protected:
        glm::vec2 m_Offset = glm::vec2(0.0f);
        bool m_IsTrigger = false;
        PhysicsMaterial* m_Material = nullptr;
    };
}
