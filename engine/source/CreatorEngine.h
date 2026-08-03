#pragma once
#include <memory>
#include <chrono>

struct GLFWwindow;

namespace Eng
{
    class Application;
    class CreatorEngine
    {
    public:
        bool Init(int windowWidth = 0, int windowHeight = 0);
        void Run();
        void Destroy();

        void SetApplication(Application* app);
        Application* GetApplication();

    private:
        std::unique_ptr<Application> m_Application;
        std::chrono::steady_clock::time_point m_LastTimePoint;
        GLFWwindow* m_Window = nullptr;
        
    };
}