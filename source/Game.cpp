#include "Game.h"
#include "GLFW/glfw3.h"
#include "GL/glew.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

bool Game::Init()
{
    m_Scene = std::make_unique<eng::Scene>("TestScene");

    GLFWwindow* window = glfwGetCurrentContext();
    glfwGetFramebufferSize(window, &m_WindowWidth, &m_WindowHeight);

    // ====== 初始化编辑器 ======
    if (!m_Editor.Init(window))
    {
        std::cerr << "编辑器初始化失败" << std::endl;
        return false;
    }

    // ====== 创建 GameView 帧缓冲 ======
    m_GameViewWidth = m_WindowWidth / 2;
    m_GameViewHeight = m_WindowHeight / 2;
    glGenFramebuffers(1, &m_GameViewFramebuffer);
    glGenTextures(1, &m_GameViewTexture);
    glBindTexture(GL_TEXTURE_2D, m_GameViewTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_GameViewWidth, m_GameViewHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, m_GameViewFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GameViewTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ====== 创建相机 ======
    m_CameraGO = m_Scene->CreateGameObject("MainCamera");
    m_Camera = m_CameraGO->AddComponent<eng::Camera>();
    m_Camera->SetOrthographic(m_GameViewHeight * 0.5f);
    m_CameraGO->GetTransform()->SetPosition(m_GameViewWidth * 0.5f, m_GameViewHeight * 0.5f, 10.0f);

    std::cout << "=== 相机系统 ===" << std::endl;
    std::cout << "正交相机: size=" << m_Camera->GetOrthoSize()
              << " pos=(" << m_CameraGO->GetTransform()->GetPosition().x
              << "," << m_CameraGO->GetTransform()->GetPosition().y << ")" << std::endl;

    // ====== 加载着色器 ======
    if (!m_SpriteShader.LoadFromFiles("source/shaders/sprite.vert",
                                      "source/shaders/sprite.frag"))
    {
        std::cerr << "精灵着色器加载失败" << std::endl;
        return false;
    }

    if (!m_GridShader.LoadFromFiles("source/shaders/grid2d.vert",
                                    "source/shaders/grid2d.frag"))
    {
        std::cerr << "网格着色器加载失败" << std::endl;
        return false;
    }

    // ====== 初始化 SpriteBatch ======
    if (!m_SpriteBatch.Init())
    {
        std::cerr << "SpriteBatch 初始化失败" << std::endl;
        return false;
    }
    m_SpriteBatch.SetShader(&m_SpriteShader);

    // ====== 创建网格大平面 ======
    SetupGridQuad();

    // ====== 创建测试纹理 ======
    CreateCheckerTexture();

    // ====== 创建精灵 ======
    auto* center = m_Scene->CreateGameObject("CenterSquare");
    auto* centerSR = center->AddComponent<eng::SpriteRenderer>();
    centerSR->SetTexture(&m_TestTexture);
    centerSR->SetSize(200.0f, 200.0f);
    centerSR->SetColor(1.0f, 0.3f, 0.3f, 1.0f);
    center->GetTransform()->SetPosition(m_GameViewWidth / 2.0f, m_GameViewHeight / 2.0f, 0.0f);

    auto* topLeft = m_Scene->CreateGameObject("TopLeft");
    auto* tlSR = topLeft->AddComponent<eng::SpriteRenderer>();
    tlSR->SetTexture(&m_TestTexture);
    tlSR->SetSize(100.0f, 100.0f);
    tlSR->SetColor(0.3f, 1.0f, 0.3f, 1.0f);
    topLeft->GetTransform()->SetPosition(150.0f, m_GameViewHeight - 150.0f, 0.0f);

    auto* bottomRight = m_Scene->CreateGameObject("BottomRight");
    auto* brSR = bottomRight->AddComponent<eng::SpriteRenderer>();
    brSR->SetTexture(&m_TestTexture);
    brSR->SetSize(120.0f, 120.0f);
    brSR->SetColor(0.3f, 0.3f, 1.0f, 1.0f);
    bottomRight->GetTransform()->SetPosition(m_GameViewWidth - 150.0f, 150.0f, 0.0f);

    std::cout << "创建了 3 个精灵" << std::endl;
    std::cout << "=======================" << std::endl;

    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
    return true;
}

void Game::SetupGridQuad()
{
    const float SIZE = 20000.0f;
    float vertices[] = {
        -SIZE, -SIZE, 0.0f,
         SIZE, -SIZE, 0.0f,
        -SIZE,  SIZE, 0.0f,
         SIZE,  SIZE, 0.0f,
    };

    glGenVertexArrays(1, &m_GridVAO);
    glGenBuffers(1, &m_GridVBO);

    glBindVertexArray(m_GridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_GridVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void Game::CreateCheckerTexture()
{
    const int texSize = 64;
    const int checkSize = texSize / 2;
    unsigned char data[texSize * texSize * 4];

    for (int y = 0; y < texSize; y++)
    {
        for (int x = 0; x < texSize; x++)
        {
            int idx = (y * texSize + x) * 4;
            bool isWhite = ((x / checkSize) + (y / checkSize)) % 2 == 0;
            unsigned char c = isWhite ? 255 : 64;
            data[idx + 0] = c;
            data[idx + 1] = c;
            data[idx + 2] = c;
            data[idx + 3] = 255;
        }
    }

    m_TestTexture.CreateFromData(texSize, texSize, data);
}

void Game::RenderToFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_GameViewFramebuffer);
    glViewport(0, 0, m_GameViewWidth, m_GameViewHeight);
    
    // 清除为红色以便调试
    glClearColor(0.5f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);  // 恢复原始清除色

    // 禁用深度测试（帧缓冲没有深度附件）
    glDisable(GL_DEPTH_TEST);

    if (m_Camera)
    {
        float aspect = (m_GameViewHeight > 0)
            ? static_cast<float>(m_GameViewWidth) / static_cast<float>(m_GameViewHeight)
            : 1.0f;

        glm::mat4 vp = m_Camera->GetViewProjectionMatrix(aspect);

        // 绘制网格
        m_GridShader.Bind();
        GLint mvpLoc = glGetUniformLocation(m_GridShader.GetID(), "u_MVP");
        if (mvpLoc != -1)
        {
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &vp[0][0]);
        }
        glBindVertexArray(m_GridVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        // 绘制精灵
        m_SpriteBatch.Begin(vp);
        m_SpriteBatch.End();
    }

    // 恢复深度测试
    glEnable(GL_DEPTH_TEST);

    // 恢复默认帧缓冲和视口
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_WindowWidth, m_WindowHeight);
}

void Game::Update(float deltaTime)
{
    if (m_Scene)
    {
        m_Scene->Update(deltaTime);
    }

    // 编辑器模式：渲染到帧缓冲 + 绘制 UI
    if (m_Editor.IsEditorMode())
    {
        // 1. 渲染场景到 GameView 帧缓冲
        RenderToFramebuffer();

        // 2. 绘制编辑器 UI
        m_Editor.BeginFrame();
        m_Editor.DrawMenuBar(m_Scene.get());
        m_Editor.DrawSceneHierarchy(m_Scene.get());
        m_Editor.DrawInspector(m_Editor.GetSelectedObject());
        m_Editor.DrawGameView(m_GameViewTexture, m_GameViewWidth, m_GameViewHeight);
        m_Editor.EndFrame();
    }
    else
    {
        // 运行时模式：直接渲染到屏幕
        glViewport(0, 0, m_WindowWidth, m_WindowHeight);
        glClear(GL_COLOR_BUFFER_BIT);

        if (!m_Camera) return;

        float aspect = (m_WindowHeight > 0)
            ? static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight)
            : 1.0f;

        glm::mat4 vp = m_Camera->GetViewProjectionMatrix(aspect);

        m_GridShader.Bind();
        GLint mvpLoc = glGetUniformLocation(m_GridShader.GetID(), "u_MVP");
        if (mvpLoc != -1)
        {
            glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &vp[0][0]);
        }
        glBindVertexArray(m_GridVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        m_SpriteBatch.Begin(vp);
        m_SpriteBatch.End();
    }
}

void Game::Destroy()
{
    m_Editor.Shutdown();
    m_Scene.reset();
    if (m_GridVAO != 0) glDeleteVertexArrays(1, &m_GridVAO);
    if (m_GridVBO != 0) glDeleteBuffers(1, &m_GridVBO);
    if (m_GameViewTexture != 0) glDeleteTextures(1, &m_GameViewTexture);
    if (m_GameViewFramebuffer != 0) glDeleteFramebuffers(1, &m_GameViewFramebuffer);
}
