#pragma once
#include <memory>
#include "input/InputManager.h"
#include <chrono>

struct GLFWwindow;

namespace eng
{
    class Application;
    class CreatorEngine
    {
    public:
        static CreatorEngine& GetInstance();

    
    private:

        CreatorEngine() = default;
        CreatorEngine(const CreatorEngine&)=delete;
        CreatorEngine(CreatorEngine&&)=delete;
        CreatorEngine& operator=(const CreatorEngine&)=delete;
        CreatorEngine& operator=(CreatorEngine&&)=delete;
        
    public:
        bool Init(int windowWidth = 0, int windowHeight = 0);
        void Run();
        void Destroy();

        void SetApplication(Application* app);
        Application* GetApplication();
        InputManager& GetInputManager();

    private:
        std::unique_ptr<Application> m_Application;
        std::chrono::steady_clock::time_point m_LastTimePoint;
        GLFWwindow* m_Window = nullptr;
        InputManager m_inputManager;
    };
}