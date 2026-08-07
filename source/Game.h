#pragma once
#include <eng.h>
#include <memory>

class Game : public eng::Application
{
    public:
        virtual bool Init() override;
        virtual void Update(float deltaTime) override;
        virtual void Destroy() override;

    private:
        void SetupGridQuad();
        void CreateCheckerTexture();
        void RenderToFramebuffer();

        // 相机
        eng::GameObject* m_CameraGO = nullptr;
        eng::Camera* m_Camera = nullptr;

        // 2D 网格
        eng::ShaderProgram m_GridShader;
        GLuint m_GridVAO = 0;
        GLuint m_GridVBO = 0;

        // 精灵渲染
        eng::SpriteBatch m_SpriteBatch;
        eng::ShaderProgram m_SpriteShader;
        eng::Texture2D m_TestTexture;

        // 编辑器
        eng::Editor m_Editor;
        GLuint m_GameViewFramebuffer = 0;
        GLuint m_GameViewTexture = 0;
        int m_GameViewWidth = 0;
        int m_GameViewHeight = 0;

        int m_WindowWidth = 0;
        int m_WindowHeight = 0;

        std::unique_ptr<eng::Scene> m_Scene;
};