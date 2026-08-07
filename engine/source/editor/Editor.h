#pragma once

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>

namespace eng
{
    class Scene;
    class GameObject;

    class Editor
    {
    public:
        Editor();
        ~Editor();

        bool Init(GLFWwindow* window);
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        void DrawSceneHierarchy(Scene* scene);
        void DrawInspector(GameObject* selected);
        void DrawGameView(GLuint framebufferTexture, int width, int height);
        void DrawMenuBar(Scene* scene);

        bool IsEditorMode() const { return m_IsEditorMode; }
        void SetEditorMode(bool mode) { m_IsEditorMode = mode; }

        GameObject* GetSelectedObject() const { return m_SelectedObject; }
        void SetSelectedObject(GameObject* obj) { m_SelectedObject = obj; }

    private:
        bool m_IsEditorMode = true;
        bool m_IsRunning = false;
        GameObject* m_SelectedObject = nullptr;
        char m_SavePath[256] = "scene.json";

        // Game View 帧缓冲
        GLuint m_Framebuffer = 0;
        GLuint m_FramebufferTexture = 0;
        int m_FramebufferWidth = 0;
        int m_FramebufferHeight = 0;

        void CreateFramebuffer(int width, int height);
        void DestroyFramebuffer();
    };
}