#include "Game.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>

bool Game::Init()
{
    m_Scene = std::make_unique<eng::Scene>("Sample Scene");

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

    m_CameraObject = m_Scene->CreateGameObject("Main Camera");
    eng::Camera* camera = m_CameraObject->AddComponent<eng::Camera>();
    camera->SetOrthographic(m_WindowHeight * 0.5f);
    m_CameraObject->GetTransform()->SetPosition(m_WindowWidth * 0.5f,
                                                m_WindowHeight * 0.5f, 10.0f);
    m_Editor.SetSceneView2DFrame(glm::vec2(m_WindowWidth * 0.5f,
                                           m_WindowHeight * 0.5f),
                                 m_WindowHeight * 0.5f);

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

    auto* center = m_Scene->CreateGameObject("Center Square");
    auto* centerRenderer = center->AddComponent<eng::SpriteRenderer>();
    centerRenderer->SetTexture(&m_TestTexture);
    centerRenderer->SetSize(200.0f, 200.0f);
    centerRenderer->SetColor(0.95f, 0.32f, 0.28f, 1.0f);
    center->GetTransform()->SetPosition(m_WindowWidth * 0.5f, m_WindowHeight * 0.5f, 0.0f);

    auto* group = m_Scene->CreateGameObject("Demo Group");
    group->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);

    auto* topLeft = m_Scene->CreateGameObject("Top Left");
    topLeft->SetParent(group, false);
    auto* topLeftRenderer = topLeft->AddComponent<eng::SpriteRenderer>();
    topLeftRenderer->SetTexture(&m_TestTexture);
    topLeftRenderer->SetSize(100.0f, 100.0f);
    topLeftRenderer->SetColor(0.28f, 0.78f, 0.48f, 1.0f);
    topLeft->GetTransform()->SetPosition(150.0f, m_WindowHeight - 150.0f, 0.0f);

    auto* bottomRight = m_Scene->CreateGameObject("Bottom Right");
    bottomRight->SetParent(group, false);
    auto* bottomRightRenderer = bottomRight->AddComponent<eng::SpriteRenderer>();
    bottomRightRenderer->SetTexture(&m_TestTexture);
    bottomRightRenderer->SetSize(120.0f, 120.0f);
    bottomRightRenderer->SetColor(0.24f, 0.50f, 0.92f, 1.0f);
    bottomRight->GetTransform()->SetPosition(m_WindowWidth - 150.0f, 150.0f, 0.0f);

    glClearColor(0.105f, 0.110f, 0.120f, 1.0f);
    return true;
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
    constexpr int textureSize = 64;
    constexpr int checkerSize = textureSize / 2;
    unsigned char data[textureSize * textureSize * 4];

    for (int y = 0; y < textureSize; ++y)
    {
        for (int x = 0; x < textureSize; ++x)
        {
            const int index = (y * textureSize + x) * 4;
            const bool light = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;
            const unsigned char value = light ? 255 : 64;
            data[index + 0] = value;
            data[index + 1] = value;
            data[index + 2] = value;
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

    if (m_Scene && m_Editor.ShouldSimulate())
    {
        m_Scene->Update(deltaTime);
    }

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
