#include "Game.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

namespace
{
    constexpr glm::vec4 kSkyColor(0.10f, 0.16f, 0.26f, 1.0f);
}

bool Game::Init()
{
    m_Scene = std::make_unique<eng::Scene>("Platform Adventure");

    GLFWwindow* window = glfwGetCurrentContext();
    glfwGetFramebufferSize(window, &m_WindowWidth, &m_WindowHeight);

    if (!m_Editor.Init(window))
    {
        std::cerr << "Failed to initialize editor" << std::endl;
        return false;
    }

    const int initialViewWidth = std::max(1, m_WindowWidth / 2);
    const int initialViewHeight = std::max(1, m_WindowHeight / 2);
    if (!m_SceneFramebuffer.Create(initialViewWidth, initialViewHeight, true) ||
        !m_GameFramebuffer.Create(initialViewWidth, initialViewHeight, true))
    {
        std::cerr << "Failed to create editor framebuffers" << std::endl;
        return false;
    }

    if (!m_SpriteShader.LoadFromFiles("source/shaders/sprite.vert",
                                      "source/shaders/sprite.frag") ||
        !m_Grid2DShader.LoadFromFiles("source/shaders/grid2d.vert",
                                      "source/shaders/grid2d.frag") ||
        !m_Grid3DShader.LoadFromFiles("source/shaders/infinite_grid.vert",
                                      "source/shaders/infinite_grid.frag"))
    {
        std::cerr << "Failed to load editor shaders" << std::endl;
        return false;
    }

    if (!m_SpriteBatch.Init())
    {
        std::cerr << "Failed to initialize SpriteBatch" << std::endl;
        return false;
    }
    m_SpriteBatch.SetShader(&m_SpriteShader);

    SetupGridQuad();
    SetupFullscreenQuad();
    CreateCheckerTexture();
    BuildPlatformGame();

    glClearColor(0.105f, 0.110f, 0.120f, 1.0f);
    return true;
}

eng::GameObject* Game::CreateBox(const char* name, const glm::vec2& position,
                                 const glm::vec2& size, const glm::vec4& color,
                                 bool dynamic, bool trigger)
{
    eng::GameObject* object = m_Scene->CreateGameObject(name);
    object->GetTransform()->SetPosition(position.x, position.y, 0.0f);
    auto* renderer = object->AddComponent<eng::SpriteRenderer>();
    renderer->SetTexture(&m_TestTexture);
    renderer->SetSize(size);
    renderer->SetColor(color);
    auto* collider = object->AddComponent<eng::BoxCollider2D>();
    collider->SetSize(size);
    collider->SetTrigger(trigger);
    if (dynamic)
    {
        auto* body = object->AddComponent<eng::Rigidbody2D>();
        body->SetMass(1.0f);
    }
    return object;
}

void Game::BuildPlatformGame()
{
    constexpr float worldWidth = 2400.0f;
    constexpr float worldHeight = 900.0f;

    m_Scene->GetPhysicsWorld()->SetGravity(glm::vec2(0.0f, -1400.0f));

    // A simple layered background keeps the sample readable without external assets.
    CreateBox("Sky", glm::vec2(worldWidth * 0.5f, worldHeight * 0.5f),
              glm::vec2(worldWidth, worldHeight), kSkyColor);
    auto* sky = m_Scene->GetGameObjects().back().get();
    sky->GetComponent<eng::BoxCollider2D>()->SetTrigger(true);
    sky->GetComponent<eng::SpriteRenderer>()->SetLayer(-100);

    m_Player = m_Scene->CreateGameObject("Player");
    m_Player->GetTransform()->SetPosition(480.0f, 180.0f, 0.0f);
    auto* playerSprite = m_Player->AddComponent<eng::SpriteRenderer>();
    playerSprite->SetTexture(&m_TestTexture);
    playerSprite->SetSize(48.0f, 72.0f);
    playerSprite->SetColor(0.25f, 0.72f, 1.0f, 1.0f);
    auto* playerBody = m_Player->AddComponent<eng::Rigidbody2D>();
    playerBody->SetMass(1.0f);
    playerBody->SetLinearDamping(10.0f);
    auto* playerCollider = m_Player->AddComponent<eng::BoxCollider2D>();
    playerCollider->SetSize(44.0f, 68.0f);
    m_PlayerController = m_Player->AddComponent<eng::CharacterController2D>();
    m_PlayerController->SetMoveSpeed(300.0f);
    m_PlayerController->SetJumpVelocity(680.0f);
    m_PlayerHealth = m_Player->AddComponent<eng::HealthComponent>(1);

    CreateBox("Ground", glm::vec2(worldWidth * 0.5f, 30.0f),
              glm::vec2(worldWidth, 60.0f), glm::vec4(0.18f, 0.23f, 0.31f, 1.0f));
    CreateBox("Platform A", glm::vec2(500.0f, 230.0f), glm::vec2(300.0f, 32.0f),
              glm::vec4(0.24f, 0.55f, 0.80f, 1.0f));
    CreateBox("Platform B", glm::vec2(900.0f, 360.0f), glm::vec2(300.0f, 32.0f),
              glm::vec4(0.30f, 0.65f, 0.48f, 1.0f));
    CreateBox("Platform C", glm::vec2(1350.0f, 260.0f), glm::vec2(280.0f, 32.0f),
              glm::vec4(0.80f, 0.48f, 0.25f, 1.0f));
    CreateBox("Platform D", glm::vec2(1750.0f, 430.0f), glm::vec2(320.0f, 32.0f),
              glm::vec4(0.54f, 0.38f, 0.82f, 1.0f));
    CreateBox("Platform E", glm::vec2(2150.0f, 300.0f), glm::vec2(300.0f, 32.0f),
              glm::vec4(0.25f, 0.70f, 0.72f, 1.0f));

    auto* moving = CreateBox("Moving Platform", glm::vec2(1120.0f, 500.0f),
                             glm::vec2(190.0f, 28.0f), glm::vec4(0.95f, 0.75f, 0.24f, 1.0f));
    auto* movingBody = moving->AddComponent<eng::Rigidbody2D>();
    movingBody->SetKinematic(true);
    auto* movingPatrol = moving->AddComponent<eng::Patrol2D>();
    movingPatrol->SetHorizontalRange(1050.0f, 1450.0f);
    movingPatrol->SetSpeed(150.0f);

    auto* enemy = CreateBox("Patrol Hazard", glm::vec2(760.0f, 105.0f),
                            glm::vec2(48.0f, 48.0f), glm::vec4(0.95f, 0.20f, 0.25f, 1.0f), false, true);
    enemy->AddComponent<eng::Hazard2D>(1);
    auto* enemyPatrol = enemy->AddComponent<eng::Patrol2D>();
    enemyPatrol->SetHorizontalRange(680.0f, 840.0f);
    enemyPatrol->SetSpeed(110.0f);

    auto* goal = CreateBox("Goal", glm::vec2(2240.0f, 380.0f),
                           glm::vec2(42.0f, 120.0f), glm::vec4(1.0f, 0.82f, 0.18f, 1.0f), false, true);
    goal->AddComponent<eng::Goal2D>();

    // A trigger below the level makes falling off the route a normal respawn.
    auto* deathPlane = CreateBox("Death Plane", glm::vec2(worldWidth * 0.5f, -90.0f),
                                 glm::vec2(worldWidth, 40.0f), glm::vec4(0.8f, 0.1f, 0.1f, 0.0f), false, true);
    deathPlane->GetComponent<eng::SpriteRenderer>()->SetLayer(-10);
    deathPlane->AddComponent<eng::Hazard2D>(1);

    m_CameraObject = m_Scene->CreateGameObject("Main Camera");
    auto* camera = m_CameraObject->AddComponent<eng::Camera>();
    camera->SetOrthographic(270.0f, -1000.0f, 1000.0f);
    m_CameraObject->GetTransform()->SetPosition(480.0f, 270.0f, 10.0f);
    auto* follow = m_CameraObject->AddComponent<eng::CameraFollow2D>();
    follow->SetTarget(m_Player);
    follow->SetBounds(glm::vec2(0.0f, 0.0f), glm::vec2(worldWidth, worldHeight));
    follow->SetSmoothing(10.0f);

    m_Scene->GetPhysicsWorld()->SetCollisionCallback(
        [this](const eng::CollisionInfo& collision) { HandleCollision(collision); });
    m_Editor.SetSceneView2DFrame(glm::vec2(480.0f, 270.0f), 270.0f);
    m_Editor.SetRuntimeStatus("PLAY: A/D or arrows move   SPACE jumps   R respawns");
}

void Game::HandleCollision(const eng::CollisionInfo& collision)
{
    if (!m_Player || !collision.colliderA || !collision.colliderB)
    {
        return;
    }
    eng::GameObject* objectA = collision.colliderA->GetGameObject();
    eng::GameObject* objectB = collision.colliderB->GetGameObject();
    eng::GameObject* other = objectA == m_Player ? objectB : (objectB == m_Player ? objectA : nullptr);
    if (!other)
    {
        return;
    }
    if (other->GetComponent<eng::Goal2D>())
    {
        m_GameWon = true;
        m_Editor.SetRuntimeStatus("YOU WIN! Press R to play again");
        if (m_PlayerController && m_PlayerController->GetRigidbody())
        {
            m_PlayerController->GetRigidbody()->SetVelocity(glm::vec2(0.0f));
        }
    }
    if (auto* hazard = other->GetComponent<eng::Hazard2D>())
    {
        if (m_PlayerHealth)
        {
            m_PlayerHealth->ApplyDamage(hazard->GetDamage());
        }
        ResetPlayer();
    }
}

void Game::ResetPlayer()
{
    if (m_PlayerController)
    {
        m_PlayerController->Respawn();
    }
    if (m_PlayerHealth)
    {
        m_PlayerHealth->Reset();
    }
    m_GameWon = false;
    m_StatusTimer = 1.0f;
}

void Game::UpdateRuntimeStatus()
{
    if (m_StatusTimer > 0.0f)
    {
        m_StatusTimer = std::max(0.0f, m_StatusTimer - 1.0f / 60.0f);
    }
}

void Game::SetupGridQuad()
{
    constexpr float size = 100000.0f;
    const float vertices[] = {
        -size, -size, 0.0f,
         size, -size, 0.0f,
        -size,  size, 0.0f,
         size,  size, 0.0f,
    };

    glGenVertexArrays(1, &m_GridVAO);
    glGenBuffers(1, &m_GridVBO);
    glBindVertexArray(m_GridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_GridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void Game::SetupFullscreenQuad()
{
    const float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f,  1.0f,
    };

    glGenVertexArrays(1, &m_FullscreenVAO);
    glGenBuffers(1, &m_FullscreenVBO);
    glBindVertexArray(m_FullscreenVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_FullscreenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glBindVertexArray(0);
}

void Game::CreateCheckerTexture()
{
    constexpr int textureSize = 2;
    unsigned char data[textureSize * textureSize * 4];

    for (int y = 0; y < textureSize; ++y)
    {
        for (int x = 0; x < textureSize; ++x)
        {
            const int index = (y * textureSize + x) * 4;
            data[index + 0] = 255;
            data[index + 1] = 255;
            data[index + 2] = 255;
            data[index + 3] = 255;
        }
    }
    m_TestTexture.CreateFromData(textureSize, textureSize, data);
}

void Game::Draw2DGrid(const glm::mat4& viewProjection)
{
    m_Grid2DShader.Bind();
    const GLint matrixLocation = glGetUniformLocation(m_Grid2DShader.GetID(), "u_MVP");
    if (matrixLocation != -1)
    {
        glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, &viewProjection[0][0]);
    }
    glBindVertexArray(m_GridVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Game::Draw3DGrid()
{
    const glm::vec3 position = m_Editor.GetSceneCameraPosition();
    const glm::vec3 forward = m_Editor.GetSceneCameraForward();
    const glm::vec3 right = m_Editor.GetSceneCameraRight();
    const glm::vec3 up = m_Editor.GetSceneCameraUp();
    const float aspect = static_cast<float>(m_SceneFramebuffer.GetWidth()) /
                         static_cast<float>(std::max(1, m_SceneFramebuffer.GetHeight()));

    m_Grid3DShader.Bind();
    glUniform3fv(glGetUniformLocation(m_Grid3DShader.GetID(), "u_CameraPos"), 1, &position.x);
    glUniform3fv(glGetUniformLocation(m_Grid3DShader.GetID(), "u_CameraForward"), 1, &forward.x);
    glUniform3fv(glGetUniformLocation(m_Grid3DShader.GetID(), "u_CameraRight"), 1, &right.x);
    glUniform3fv(glGetUniformLocation(m_Grid3DShader.GetID(), "u_CameraUp"), 1, &up.x);
    glUniform1f(glGetUniformLocation(m_Grid3DShader.GetID(), "u_FOV"), m_Editor.GetSceneCameraFov());
    glUniform1f(glGetUniformLocation(m_Grid3DShader.GetID(), "u_AspectRatio"), aspect);
    glBindVertexArray(m_FullscreenVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Game::RenderSceneView()
{
    m_SceneFramebuffer.Bind();
    glClearColor(0.105f, 0.110f, 0.120f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    const float aspect = static_cast<float>(m_SceneFramebuffer.GetWidth()) /
                         static_cast<float>(std::max(1, m_SceneFramebuffer.GetHeight()));
    const glm::mat4 viewProjection = m_Editor.GetSceneViewProjectionMatrix(aspect);

    glDisable(GL_DEPTH_TEST);
    if (m_Editor.IsSceneView3D())
    {
        Draw3DGrid();
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        Draw2DGrid(viewProjection);
    }

    m_SpriteBatch.Begin(viewProjection);
    m_SpriteBatch.End();

    glDisable(GL_DEPTH_TEST);
    eng::Framebuffer::Unbind();
}

eng::Camera* Game::FindGameCamera() const
{
    if (!m_Scene)
    {
        return nullptr;
    }
    for (const auto& object : m_Scene->GetGameObjects())
    {
        if (object && object->IsActive())
        {
            if (eng::Camera* camera = object->GetComponent<eng::Camera>())
            {
                return camera;
            }
        }
    }
    return nullptr;
}

void Game::RenderGameView()
{
    m_GameFramebuffer.Bind();
    glClearColor(0.075f, 0.080f, 0.090f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if (eng::Camera* camera = FindGameCamera())
    {
        const float aspect = static_cast<float>(m_GameFramebuffer.GetWidth()) /
                             static_cast<float>(std::max(1, m_GameFramebuffer.GetHeight()));
        if (camera->GetProjectionType() == eng::Camera::ProjectionType::Perspective)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);

        m_SpriteBatch.Begin(camera->GetViewProjectionMatrix(aspect));
        m_SpriteBatch.End();
    }

    glDisable(GL_DEPTH_TEST);
    eng::Framebuffer::Unbind();
}

void Game::Update(float deltaTime)
{
    GLFWwindow* window = glfwGetCurrentContext();
    glfwGetFramebufferSize(window, &m_WindowWidth, &m_WindowHeight);

    eng::InputManager& input = eng::CreatorEngine::GetInstance().GetInputManager();
    const bool resetPressed = input.IsKeyPressed(GLFW_KEY_R);
    if (resetPressed && !m_PreviousResetPressed)
    {
        ResetPlayer();
        m_Editor.SetRuntimeStatus("PLAY: A/D or arrows move   SPACE jumps   R respawns");
    }
    m_PreviousResetPressed = resetPressed;

    if (m_Scene && m_Editor.ShouldSimulate() && !m_GameWon)
    {
        m_Scene->Update(std::min(deltaTime, 1.0f / 20.0f));
    }
    UpdateRuntimeStatus();

    const glm::ivec2 sceneSize = m_Editor.GetSceneViewportSize();
    const glm::ivec2 gameSize = m_Editor.GetGameViewportSize();
    m_SceneFramebuffer.Resize(std::clamp(sceneSize.x, 1, 4096),
                              std::clamp(sceneSize.y, 1, 4096));
    m_GameFramebuffer.Resize(std::clamp(gameSize.x, 1, 4096),
                             std::clamp(gameSize.y, 1, 4096));

    RenderSceneView();
    RenderGameView();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
    glClearColor(0.105f, 0.110f, 0.120f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_Editor.BeginFrame();
    m_Editor.DrawLayout(m_Scene.get(),
                        m_SceneFramebuffer.GetColorAttachment(),
                        m_SceneFramebuffer.GetWidth(), m_SceneFramebuffer.GetHeight(),
                        m_GameFramebuffer.GetColorAttachment(),
                        m_GameFramebuffer.GetWidth(), m_GameFramebuffer.GetHeight());
    m_Editor.EndFrame();
}

void Game::Destroy()
{
    m_Scene.reset();
    eng::MonoRuntime::GetInstance().Shutdown();
    m_SceneFramebuffer.Destroy();
    m_GameFramebuffer.Destroy();
    if (m_GridVAO != 0) glDeleteVertexArrays(1, &m_GridVAO);
    if (m_GridVBO != 0) glDeleteBuffers(1, &m_GridVBO);
    if (m_FullscreenVAO != 0) glDeleteVertexArrays(1, &m_FullscreenVAO);
    if (m_FullscreenVBO != 0) glDeleteBuffers(1, &m_FullscreenVBO);
    m_GridVAO = 0;
    m_GridVBO = 0;
    m_FullscreenVAO = 0;
    m_FullscreenVBO = 0;
    m_Editor.Shutdown();
}
