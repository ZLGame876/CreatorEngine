#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace eng
{
    class Component;
    class GameObject;
    class Scene;

    class Editor
    {
    public:
        enum class SceneViewMode
        {
            Mode2D,
            Mode3D
        };

        enum class PlayState
        {
            Editing,
            Playing,
            Paused
        };

        Editor();
        ~Editor();

        bool Init(GLFWwindow* window);
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        void DrawLayout(Scene* scene,
                        GLuint sceneTexture, int sceneTextureWidth, int sceneTextureHeight,
                        GLuint gameTexture, int gameTextureWidth, int gameTextureHeight);

        bool IsEditorMode() const { return m_IsEditorMode; }
        void SetEditorMode(bool mode) { m_IsEditorMode = mode; }

        GameObject* GetSelectedObject() const { return m_SelectedObject; }
        void SetSelectedObject(GameObject* object) { m_SelectedObject = object; }

        SceneViewMode GetSceneViewMode() const { return m_SceneViewMode; }
        bool IsSceneView3D() const { return m_SceneViewMode == SceneViewMode::Mode3D; }
        PlayState GetPlayState() const { return m_PlayState; }
        bool ShouldSimulate() const { return m_PlayState == PlayState::Playing; }

        glm::ivec2 GetSceneViewportSize() const { return m_SceneViewportSize; }
        glm::ivec2 GetGameViewportSize() const { return m_GameViewportSize; }

        glm::mat4 GetSceneViewMatrix() const;
        glm::mat4 GetSceneProjectionMatrix(float aspectRatio) const;
        glm::mat4 GetSceneViewProjectionMatrix(float aspectRatio) const;
        glm::vec3 GetSceneCameraPosition() const;
        glm::vec3 GetSceneCameraForward() const;
        glm::vec3 GetSceneCameraRight() const;
        glm::vec3 GetSceneCameraUp() const;
        float GetSceneCameraFov() const { return m_SceneFov; }

        void SetSceneView2DFrame(const glm::vec2& center, float orthoSize);

    private:
        bool m_IsEditorMode = true;
        bool m_IsInitialized = false;
        bool m_IsShutdown = true;
        bool m_ShowHierarchy = true;
        bool m_ShowInspector = true;
        bool m_ShowProject = true;
        bool m_ShowConsole = true;
        GameObject* m_SelectedObject = nullptr;
        GameObject* m_PendingDelete = nullptr;
        GameObject* m_PendingReparentObject = nullptr;
        GameObject* m_PendingParent = nullptr;
        Component* m_PendingRemoveComponent = nullptr;

        PlayState m_PlayState = PlayState::Editing;
        SceneViewMode m_SceneViewMode = SceneViewMode::Mode2D;
        glm::ivec2 m_SceneViewportSize = glm::ivec2(640, 360);
        glm::ivec2 m_GameViewportSize = glm::ivec2(640, 360);
        glm::vec2 m_Scene2DCenter = glm::vec2(0.0f);
        float m_Scene2DOrthoSize = 10.0f;
        glm::vec3 m_Scene3DPivot = glm::vec3(0.0f);
        float m_Scene3DYaw = 45.0f;
        float m_Scene3DPitch = 32.0f;
        float m_Scene3DDistance = 14.0f;
        float m_SceneFov = 60.0f;

        char m_SavePath[256] = "scene.json";
        std::vector<std::string> m_ConsoleMessages;

        void ApplyStyle();
        void DrawMenuBar(Scene* scene);
        void DrawToolbar();
        void DrawSceneHierarchy(Scene* scene);
        void DrawGameObjectNode(GameObject* object, Scene* scene);
        void DrawInspector(GameObject* selected);
        void DrawComponentInspector(GameObject* selected, Component* component);
        void DrawAddComponentMenu(GameObject* selected);
        void DrawViewportTabs(GLuint sceneTexture, int sceneTextureWidth, int sceneTextureHeight,
                              GLuint gameTexture, int gameTextureWidth, int gameTextureHeight);
        void DrawSceneView(GLuint texture, int width, int height);
        void DrawGameView(GLuint texture, int width, int height);
        void DrawBottomPanel();
        void DrawProjectPanel();
        void DrawConsolePanel();
        void HandleSceneViewInput();
        void FocusSelection();
        void ProcessDeferredActions(Scene* scene);
        void AddConsoleMessage(const std::string& message);
    };
}
