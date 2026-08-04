#include "Game.h"
#include "GLFW/glfw3.h"
#include <iostream>


bool Game::Init()
{
    //初始化游戏
    return true;
}

void Game::Update(float deltaTime)
{
    auto& input = eng::CreatorEngine::GetInstance().GetInputManager();
    if (input.IsKeyPressed(GLFW_KEY_A))
    {
        std::cout<< "[A] button is pressed"<<std::endl;
    }
}

void Game::Destroy()
{
    //销毁游戏
}
