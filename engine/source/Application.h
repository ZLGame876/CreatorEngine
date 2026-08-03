#pragma once



namespace Eng
{
    class Application
    {
        public:
            virtual ~Application() = default;

            virtual bool Init() = 0;
            //增量时间以秒为单位
            virtual void Update(float deltaTime) = 0;
            virtual void Destroy() = 0;

            void SetNeedsToBeClosed(bool value);
            bool NeedsToBeClosed() const;

        private:
            bool m_NeedsToBeClosed = false;
    };
}