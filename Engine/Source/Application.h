#pragma once



namespace Eng
{
    class Application
    {
        public:
            virtual bool Init() = 0;
            virtual void Run() = 0;
            virtual void Destroy() = 0;
    };
}