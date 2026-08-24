#include "physics/Collider2D.h"
#include "physics/Rigidbody2D.h"
#include "physics/PhysicsWorld.h"
#include "core/Transform.h"
#include "core/GameObject.h"
#include "core/Scene.h"

namespace eng
{
    Collider2D::Collider2D()
    {
    }

    Collider2D::~Collider2D()
    {
        // 从 PhysicsWorld 注销
        GameObject* go = GetGameObject();
        if (go)
        {
            Scene* scene = go->GetScene();
            if (scene && scene->GetPhysicsWorld())
            {
                scene->GetPhysicsWorld()->RemoveCollider(this);
            }
        }
    }

    void Collider2D::Awake()
    {
        // 注册到 PhysicsWorld
        GameObject* go = GetGameObject();
        if (go)
        {
            Scene* scene = go->GetScene();
            if (scene && scene->GetPhysicsWorld())
            {
                scene->GetPhysicsWorld()->AddCollider(this);
            }
        }
    }

    Rigidbody2D* Collider2D::GetAttachedRigidbody() const
    {
        GameObject* go = GetGameObject();
        if (!go) return nullptr;
        return go->GetComponent<Rigidbody2D>();
    }

    glm::vec2 Collider2D::GetWorldCenter() const
    {
        Transform* transform = GetTransform();
        if (!transform) return glm::vec2(0.0f);

        glm::vec3 pos = transform->GetPosition();
        return glm::vec2(pos.x, pos.y) + m_Offset;
    }

    nlohmann::json Collider2D::Serialize() const
    {
        nlohmann::json json;
        json["type"] = GetClassName();
        json["offset"] = { m_Offset.x, m_Offset.y };
        json["isTrigger"] = m_IsTrigger;
        return json;
    }

    void Collider2D::Deserialize(const nlohmann::json& json)
    {
        if (json.contains("offset"))
        {
            auto o = json["offset"];
            m_Offset = glm::vec2(o[0].get<float>(), o[1].get<float>());
        }
        if (json.contains("isTrigger"))
            m_IsTrigger = json["isTrigger"].get<bool>();
    }
}
