#include "graphics/SpriteRenderer.h"
#include "graphics/SpriteBatch.h"

namespace eng
{
    void SpriteRenderer::Awake()
    {
        SpriteBatch::RegisterSprite(this);
    }

    void SpriteRenderer::OnDestroy()
    {
        SpriteBatch::UnregisterSprite(this);
    }

    void SpriteRenderer::SetNativeSize()
    {
        if (m_Texture && m_Texture->IsValid())
        {
            m_Size = glm::vec2(
                static_cast<float>(m_Texture->GetWidth()),
                static_cast<float>(m_Texture->GetHeight())
            );
        }
    }

    nlohmann::json SpriteRenderer::Serialize() const
    {
        nlohmann::json data;
        data["color"] = {m_Color.x, m_Color.y, m_Color.z, m_Color.w};
        data["size"] = {m_Size.x, m_Size.y};
        data["layer"] = m_Layer;
        data["orderInLayer"] = m_OrderInLayer;
        return data;
    }

    void SpriteRenderer::Deserialize(const nlohmann::json& json)
    {
        if (json.contains("color"))
        {
            auto& c = json["color"];
            m_Color = glm::vec4(c[0].get<float>(), c[1].get<float>(), c[2].get<float>(), c[3].get<float>());
        }
        if (json.contains("size"))
        {
            auto& s = json["size"];
            m_Size = glm::vec2(s[0].get<float>(), s[1].get<float>());
        }
        if (json.contains("layer"))
            m_Layer = json["layer"].get<int>();
        if (json.contains("orderInLayer"))
            m_OrderInLayer = json["orderInLayer"].get<int>();
    }
}