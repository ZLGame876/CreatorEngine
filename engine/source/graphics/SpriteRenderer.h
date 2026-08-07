#pragma once

#include "core/Component.h"
#include "graphics/Texture.h"
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace eng
{
    class SpriteRenderer : public Component
    {
    public:
        SpriteRenderer() = default;

        // 纹理
        void SetTexture(Texture2D* texture) { m_Texture = texture; }
        Texture2D* GetTexture() const { return m_Texture; }

        // 颜色
        void SetColor(const glm::vec4& color) { m_Color = color; }
        void SetColor(float r, float g, float b, float a) { m_Color = glm::vec4(r, g, b, a); }
        const glm::vec4& GetColor() const { return m_Color; }

        // 尺寸（世界单位，默认 1x1）
        void SetSize(const glm::vec2& size) { m_Size = size; }
        void SetSize(float w, float h) { m_Size = glm::vec2(w, h); }
        const glm::vec2& GetSize() const { return m_Size; }

        // 渲染排序层
        void SetLayer(int layer) { m_Layer = layer; }
        int GetLayer() const { return m_Layer; }
        void SetOrderInLayer(int order) { m_OrderInLayer = order; }
        int GetOrderInLayer() const { return m_OrderInLayer; }

        // 自动适配纹理尺寸
        void SetNativeSize();

        // 序列化
        nlohmann::json Serialize() const override;
        void Deserialize(const nlohmann::json& json) override;

        // 生命周期
        void Awake() override;
        void OnDestroy() override;

        const char* GetClassName() const override { return "SpriteRenderer"; }

    private:
        Texture2D* m_Texture = nullptr;
        glm::vec4 m_Color = glm::vec4(1.0f);
        glm::vec2 m_Size = glm::vec2(1.0f, 1.0f);
        int m_Layer = 0;
        int m_OrderInLayer = 0;
    };
}