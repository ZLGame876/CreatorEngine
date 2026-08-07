#include "graphics/SpriteBatch.h"
#include "core/GameObject.h"
#include "core/Transform.h"
#include <algorithm>
#include <iostream>

namespace eng
{
    std::vector<SpriteRenderer*> SpriteBatch::s_AllSprites;

    SpriteBatch::~SpriteBatch()
    {
        if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO) glDeleteBuffers(1, &m_VBO);
        if (m_EBO) glDeleteBuffers(1, &m_EBO);
    }

    bool SpriteBatch::Init()
    {
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        std::vector<GLuint> indices(MAX_INDICES);
        for (int i = 0; i < MAX_QUADS; i++)
        {
            indices[i * 6 + 0] = i * 4 + 0;
            indices[i * 6 + 1] = i * 4 + 1;
            indices[i * 6 + 2] = i * 4 + 2;
            indices[i * 6 + 3] = i * 4 + 2;
            indices[i * 6 + 4] = i * 4 + 3;
            indices[i * 6 + 5] = i * 4 + 0;
        }
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

        glBindVertexArray(0);

        m_Vertices.resize(MAX_VERTICES);

        std::cout << "SpriteBatch 初始化完成 (最大 " << MAX_QUADS << " 个精灵)" << std::endl;
        return true;
    }

    void SpriteBatch::RegisterSprite(SpriteRenderer* sprite)
    {
        s_AllSprites.push_back(sprite);
    }

    void SpriteBatch::UnregisterSprite(SpriteRenderer* sprite)
    {
        auto it = std::find(s_AllSprites.begin(), s_AllSprites.end(), sprite);
        if (it != s_AllSprites.end())
        {
            s_AllSprites.erase(it);
        }
    }

    void SpriteBatch::Begin(const glm::mat4& viewProj)
    {
        m_ViewProj = viewProj;
        m_QuadCount = 0;
        m_Sprites.clear();

        for (auto* sprite : s_AllSprites)
        {
            if (sprite->GetGameObject() && sprite->GetGameObject()->IsActive())
            {
                m_Sprites.push_back(sprite);
            }
        }

        std::sort(m_Sprites.begin(), m_Sprites.end(),
            [](const SpriteRenderer* a, const SpriteRenderer* b) {
                if (a->GetLayer() != b->GetLayer())
                    return a->GetLayer() < b->GetLayer();
                return a->GetOrderInLayer() < b->GetOrderInLayer();
            });
    }

    void SpriteBatch::End()
    {
        Flush();
    }

    void SpriteBatch::Flush()
    {
        if (m_Sprites.empty() || !m_Shader) return;

        m_QuadCount = 0;

        // 生成所有精灵的顶点
        for (auto* sprite : m_Sprites)
        {
            if (m_QuadCount >= MAX_QUADS) break;
            if (!sprite || !sprite->GetTexture() || !sprite->GetTexture()->IsValid()) continue;

            Transform* transform = sprite->GetTransform();
            if (!transform) continue;

            glm::mat4 model = transform->GetLocalToWorldMatrix();
            glm::vec2 size = sprite->GetSize();
            glm::vec4 color = sprite->GetColor();

            float hw = size.x * 0.5f;
            float hh = size.y * 0.5f;

            glm::vec4 corners[4] = {
                model * glm::vec4(-hw, -hh, 0.0f, 1.0f),
                model * glm::vec4( hw, -hh, 0.0f, 1.0f),
                model * glm::vec4( hw,  hh, 0.0f, 1.0f),
                model * glm::vec4(-hw,  hh, 0.0f, 1.0f),
            };

            glm::vec2 uvs[4] = {
                glm::vec2(0.0f, 0.0f),
                glm::vec2(1.0f, 0.0f),
                glm::vec2(1.0f, 1.0f),
                glm::vec2(0.0f, 1.0f),
            };

            for (int i = 0; i < 4; i++)
            {
                auto& v = m_Vertices[m_QuadCount * 4 + i];
                v.position = glm::vec3(corners[i]);
                v.texCoord = uvs[i];
                v.color = color;
            }

            m_QuadCount++;
        }

        if (m_QuadCount == 0) return;

        m_Shader->Bind();

        GLint mvpLoc = glGetUniformLocation(m_Shader->GetID(), "u_MVP");
        if (mvpLoc != -1)
        {
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &m_ViewProj[0][0]);
        }

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_QuadCount * 4 * sizeof(Vertex), m_Vertices.data());

        // 按纹理分组绘制
        GLuint currentTex = 0;
        int batchStart = 0;
        int batchCount = 0;

        for (int i = 0; i < m_QuadCount; i++)
        {
            GLuint texID = m_Sprites[i]->GetTexture()->GetID();

            if (texID != currentTex && batchCount > 0)
            {
                glBindTexture(GL_TEXTURE_2D, currentTex);
                glDrawElements(GL_TRIANGLES, batchCount * 6, GL_UNSIGNED_INT,
                    (void*)(batchStart * 6 * sizeof(GLuint)));
                batchStart = i;
                batchCount = 0;
            }

            currentTex = texID;
            batchCount++;
        }

        // 最后一批
        if (batchCount > 0)
        {
            glBindTexture(GL_TEXTURE_2D, currentTex);
            glDrawElements(GL_TRIANGLES, batchCount * 6, GL_UNSIGNED_INT,
                (void*)(batchStart * 6 * sizeof(GLuint)));
        }

        glBindVertexArray(0);
    }
}