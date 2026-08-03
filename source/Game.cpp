#include "Game.h"
#include <iostream>


bool Game::Init()
{
    //初始化游戏
    return true;
}

void Game::Update(float deltaTime)
{
    //更新游戏
    std::cout << "Current deltaTime: " << deltaTime << std::endl;
}

void Game::Destroy()
{
    //销毁游戏
}
