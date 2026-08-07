#include "physics/CircleCollider2D.h"
#include "physics/BoxCollider2D.h"
#include "core/Transform.h"
#include <cmath>

namespace eng
{
    CircleCollider2D::CircleCollider2D()
    {
    }

    void CircleCollider2D::GetAABB(glm::vec2& min, glm::vec2& max) const
    {
        glm::vec2 center = GetWorldCenter();

        // 考虑缩放
        float scaledRadius = m_Radius;
        Transform* transform = GetTransform();
        if (transform)
        {
            glm::vec3 scale = transform->GetScale();
            float maxScale = std::max(std::abs(scale.x), std::abs(scale.y));
            scaledRadius *= maxScale;
        }

        min = center - glm::vec2(scaledRadius);
        max = center + glm::vec2(scaledRadius);
    }

    bool CircleCollider2D::TestCollision(const Collider2D* other, glm::vec2& normal, float& depth) const
    {
        // Circle vs Circle
        if (const CircleCollider2D* circle = dynamic_cast<const CircleCollider2D*>(other))
        {
            glm::vec2 centerA = GetWorldCenter();
            glm::vec2 centerB = circle->GetWorldCenter();

            // 考虑缩放
            float radiusA = m_Radius;
            float radiusB = circle->m_Radius;

            Transform* transformA = GetTransform();
            if (transformA)
            {
                glm::vec3 scale = transformA->GetScale();
                radiusA *= std::max(std::abs(scale.x), std::abs(scale.y));
            }

            Transform* transformB = circle->GetTransform();
            if (transformB)
            {
                glm::vec3 scale = transformB->GetScale();
                radiusB *= std::max(std::abs(scale.x), std::abs(scale.y));
            }

            glm::vec2 diff = centerB - centerA;
            float distSq = glm::dot(diff, diff);
            float totalRadius = radiusA + radiusB;

            if (distSq > totalRadius * totalRadius) return false;

            float dist = std::sqrt(distSq);
            if (dist > 0.0001f)
            {
                normal = diff / dist;
                depth = totalRadius - dist;
            }
            else
            {
                normal = glm::vec2(1.0f, 0.0f);
                depth = totalRadius;
            }

            return true;
        }

        // Circle vs Box (委托给Box处理，然后反转法线)
        if (const BoxCollider2D* box = dynamic_cast<const BoxCollider2D*>(other))
        {
            glm::vec2 reverseNormal;
            bool collided = box->TestCollision(this, reverseNormal, depth);
            if (collided)
            {
                normal = -reverseNormal;
                return true;
            }
            return false;
        }

        return false;
    }

    nlohmann::json CircleCollider2D::Serialize() const
    {
        nlohmann::json json = Collider2D::Serialize();
        json["radius"] = m_Radius;
        return json;
    }

    void CircleCollider2D::Deserialize(const nlohmann::json& json)
    {
        Collider2D::Deserialize(json);
        if (json.contains("radius"))
            m_Radius = json["radius"].get<float>();
    }
}
