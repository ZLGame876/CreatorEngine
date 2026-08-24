#pragma once

#include <eng.h>
#include <memory>

class Game : public eng::Application
{
public:
    bool Init() override;
    void Update(float deltaTime) override;
    void Destroy() override;

private:
    void BuildPlatformGame();
    eng::GameObject* CreateBox(const char* name, const glm::vec2& position,
                               const glm::vec2& size, const glm::vec4& color,
                               bool dynamic = false, bool trigger = false);
    void HandleCollision(const eng::CollisionInfo& collision);
    void ResetPlayer();
    void UpdateRuntimeStatus();
    void SetupGridQuad();
    void SetupFullscreenQuad();
    void CreateCheckerTexture();
    void RenderSceneView();
    void RenderGameView();
    void Draw2DGrid(const glm::mat4& viewProjection);
    void Draw3DGrid();
    eng::Camera* FindGameCamera() const;

    eng::GameObject* m_CameraObject = nullptr;

    eng::ShaderProgram m_Grid2DShader;
    GLuint m_GridVAO = 0;
    GLuint m_GridVBO = 0;

    eng::ShaderProgram m_Grid3DShader;
    GLuint m_FullscreenVAO = 0;
    GLuint m_FullscreenVBO = 0;

    eng::SpriteBatch m_SpriteBatch;
    eng::ShaderProgram m_SpriteShader;
    eng::Texture2D m_TestTexture;

    eng::GameObject* m_Player = nullptr;
    eng::CharacterController2D* m_PlayerController = nullptr;
    eng::HealthComponent* m_PlayerHealth = nullptr;
    bool m_GameWon = false;
    bool m_PreviousResetPressed = false;
    float m_StatusTimer = 0.0f;

    eng::Editor m_Editor;
    eng::Framebuffer m_SceneFramebuffer;
    eng::Framebuffer m_GameFramebuffer;

    int m_WindowWidth = 0;
    int m_WindowHeight = 0;
    std::unique_ptr<eng::Scene> m_Scene;
};
