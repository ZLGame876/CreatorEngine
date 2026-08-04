#include "CreatorEngine.h"
#include "Application.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace eng
{

    void keyCallback(GLFWwindow* window, int key, int, int action, int)
    {
        auto& inputManager = eng::CreatorEngine::GetInstance().GetInputManager();
        if (action == GLFW_PRESS)
        {
            inputManager.SetKeyPressed(key, true);
        }
        else if (action == GLFW_RELEASE)
        {
            inputManager.SetKeyPressed(key, false);
        }
    }

    CreatorEngine& CreatorEngine:: GetInstance()
    {
        static CreatorEngine instance;
        return instance;
    }

    bool CreatorEngine::Init(int windowWidth, int windowHeight)
     {
        if(!m_Application)
        {
            return false;
        }

        if(!glfwInit())
        {
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//设置OpenGL上下文版本为3.3
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//设置OpenGL上下文次要版本为3
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//设置OpenGL上下文为核心模式

        //查询主显示器
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        //参数为0时自动取屏幕的80%，否则用调用方指定尺寸
        if(windowWidth <= 0)
        {
            windowWidth = mode->width * 0.8;
        }
        if(windowHeight <= 0)
        {
            windowHeight = mode->height * 0.8;
        }

        //创建窗口，标题为"Creator Engine"
        m_Window = glfwCreateWindow(windowWidth, windowHeight, "Creator Engine", nullptr, nullptr);

        //如果窗口创建失败，输出错误信息并终止程序
        if(!m_Window)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwSetKeyCallback(m_Window, keyCallback);

        //设置窗口位置，居中显示
        glfwSetWindowPos(m_Window, (mode->width - windowWidth) / 2, (mode->height - windowHeight) / 2);
        glfwMakeContextCurrent(m_Window);//设置当前上下文为新创建的窗口

        //初始化GLEW,如果初始化失败，输出错误信息并终止程序
        if(glewInit() != GLEW_OK)
        {
            std::cerr << "Failed to initialize GLEW" << std::endl;
            glfwTerminate();
            return false;
        }

        return m_Application->Init();
    }
    void CreatorEngine::Run()
    {
        if(!m_Application)
        {
            return;
        }

        m_LastTimePoint = std::chrono::high_resolution_clock::now();

        while(!m_Application->NeedsToBeClosed() && !glfwWindowShouldClose(m_Window))
        {
            auto now = std::chrono::high_resolution_clock::now();
            //计算增量时间
            float deltaTime = std::chrono::duration<float>(now - m_LastTimePoint).count();
            m_LastTimePoint = now;

            m_Application->Update(deltaTime);

            glfwSwapBuffers(m_Window);//交换前后缓冲区
            glfwPollEvents();//处理窗口事件
        }
    }
    void CreatorEngine::Destroy()
    {
        if(m_Application)
        {
            m_Application->Destroy();
            m_Application.reset();
        }
        glfwDestroyWindow(m_Window);//销毁窗口
        glfwTerminate();//终止GLFW，释放资源
    }

    void CreatorEngine::SetApplication(Application* app)
    {
        m_Application.reset(app);
    }
    Application* CreatorEngine::GetApplication()
    {
        return m_Application.get();
    }

    InputManager& CreatorEngine::GetInputManager()
    {
        return m_inputManager;
    }

}