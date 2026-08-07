#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstddef>
#include <vector>
#include "graphics/SpriteRenderer.h"
#include "graphics/shaderprogram.h"

namespace eng
{
    class SpriteBatch
    {
    public:
        SpriteBatch() = default;
        ~SpriteBatch();

        SpriteBatch(const SpriteBatch&) = delete;
        SpriteBatch& operator=(const SpriteBatch&) = delete;

        // 初始化
        bool Init();

        // 渲染一帧
        void Begin(const glm::mat4& viewProj);
        void End();
        void Flush();

        // 注册/注销 SpriteRenderer（由 SpriteRenderer 生命周期调用）
        static void RegisterSprite(SpriteRenderer* sprite);
        static void UnregisterSprite(SpriteRenderer* sprite);

        // 设置着色器
        void SetShader(ShaderProgram* shader) { m_Shader = shader; }

    private:
        struct Vertex
        {
            glm::vec3 position;
            glm::vec2 texCoord;
            glm::vec4 color;
        };

        static const int MAX_QUADS = 10000;
        static const int MAX_VERTICES = MAX_QUADS * 4;
        static const int MAX_INDICES  = MAX_QUADS * 6;

        GLuint m_VAO = 0;
        GLuint m_VBO = 0;
        GLuint m_EBO = 0;

        ShaderProgram* m_Shader = nullptr;
        glm::mat4 m_ViewProj = glm::mat4(1.0f);

        std::vector<SpriteRenderer*> m_Sprites;  // 本帧要渲染的精灵
        std::vector<Vertex> m_Vertices;
        int m_QuadCount = 0;

        // 静态注册表
        static std::vector<SpriteRenderer*> s_AllSprites;
    };
}