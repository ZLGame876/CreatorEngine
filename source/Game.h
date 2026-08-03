#pragma once
#include <eng.h>

class Game : public Eng::Application
{
    public:
        virtual bool Init() override;
        virtual void Update(float deltaTime) override;
        virtual void Destroy() override;
};