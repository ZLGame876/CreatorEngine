#include "physics/BoxCollider2D.h"
#include "physics/CircleCollider2D.h"
#include "core/Transform.h"
#include <algorithm>
#include <cmath>

namespace eng
{
    BoxCollider2D::BoxCollider2D()
    {
    }

    void BoxCollider2D::GetAABB(glm::vec2& min, glm::vec2& max) const
    {
        glm::vec2 center = GetWorldCenter();
        glm::vec2 halfSize = m_Size * 0.5f;

        // 考虑缩放
        Transform* transform = GetTransform();
        if (transform)
        {
            glm::vec3 scale = transform->GetScale();
            halfSize.x *= std::abs(scale.x);
            halfSize.y *= std::abs(scale.y);
        }

        min = center - halfSize;
        max = center + halfSize;
    }

    bool BoxCollider2D::TestCollision(const Collider2D* other, glm::vec2& normal, float& depth) const
    {
        // Box vs Box (AABB)
        if (const BoxCollider2D* box = dynamic_cast<const BoxCollider2D*>(other))
        {
            glm::vec2 minA, maxA, minB, maxB;
            GetAABB(minA, maxA);
            box->GetAABB(minB, maxB);

            // 检查分离轴
            if (maxA.x < minB.x || minA.x > maxB.x) return false;
            if (maxA.y < minB.y || minA.y > maxB.y) return false;

            // 计算重叠
            float overlapX = std::min(maxA.x, maxB.x) - std::max(minA.x, minB.x);
            float overlapY = std::min(maxA.y, maxB.y) - std::max(minA.y, minB.y);

            // 选择最小穿透轴作为法线
            if (overlapX < overlapY)
            {
                depth = overlapX;
                // Collision normals consistently point from collider A to collider B.
                normal = glm::vec2((GetWorldCenter().x < box->GetWorldCenter().x) ? 1.0f : -1.0f, 0.0f);
            }
            else
            {
                depth = overlapY;
                normal = glm::vec2(0.0f, (GetWorldCenter().y < box->GetWorldCenter().y) ? 1.0f : -1.0f);
            }

            return true;
        }

        // Box vs Circle
        if (const CircleCollider2D* circle = dynamic_cast<const CircleCollider2D*>(other))
        {
            glm::vec2 min, max;
            GetAABB(min, max);

            glm::vec2 circleCenter = circle->GetWorldCenter();
            float radius = circle->GetRadius();

            // 找到盒子上离圆心最近的点
            glm::vec2 closest = glm::clamp(circleCenter, min, max);
            glm::vec2 diff = circleCenter - closest;
            float distSq = glm::dot(diff, diff);

            if (distSq > radius * radius) return false;

            float dist = std::sqrt(distSq);
            if (dist > 0.0001f)
            {
                normal = diff / dist;
                depth = radius - dist;
            }
            else
            {
                // 圆心在盒子内部
                normal = glm::vec2(0.0f, 1.0f);
                depth = radius;
            }

            return true;
        }

        return false;
    }

    nlohmann::json BoxCollider2D::Serialize() const
    {
        nlohmann::json json = Collider2D::Serialize();
        json["size"] = { m_Size.x, m_Size.y };
        return json;
    }

    void BoxCollider2D::Deserialize(const nlohmann::json& json)
    {
        Collider2D::Deserialize(json);
        if (json.contains("size"))
        {
            auto s = json["size"];
            m_Size = glm::vec2(s[0].get<float>(), s[1].get<float>());
        }
    }
}
