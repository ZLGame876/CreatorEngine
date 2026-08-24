#pragma once

#include <memory>
#include <string>

namespace eng
{
    class CSharpScript;

    class MonoRuntime
    {
    public:
        static MonoRuntime& GetInstance();

        ~MonoRuntime();

        MonoRuntime(const MonoRuntime&) = delete;
        MonoRuntime& operator=(const MonoRuntime&) = delete;

        bool Initialize(const std::string& domainName = "CreatorEngine");
        void Shutdown();

        bool IsAvailable() const;
        bool IsInitialized() const;
        const std::string& GetLastError() const { return m_LastError; }

        bool CreateScriptInstance(CSharpScript& script);
        void DestroyScriptInstance(CSharpScript& script);
        bool InvokeUpdate(CSharpScript& script, float deltaTime);

    private:
        MonoRuntime();

        struct Impl;
        std::unique_ptr<Impl> m_Impl;
        std::string m_LastError;
    };
}
