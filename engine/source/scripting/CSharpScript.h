#pragma once

#include "core/Script.h"
#include <string>

namespace eng
{
    class CSharpScript : public Script
    {
    public:
        CSharpScript() = default;
        ~CSharpScript() override = default;

        void SetAssemblyPath(const std::string& path) { m_AssemblyPath = path; }
        const std::string& GetAssemblyPath() const { return m_AssemblyPath; }

        void SetNamespaceName(const std::string& name) { m_NamespaceName = name; }
        const std::string& GetNamespaceName() const { return m_NamespaceName; }

        void SetClassName(const std::string& name) { m_ClassName = name; }
        const std::string& GetManagedClassName() const { return m_ClassName; }

        bool IsLoaded() const { return m_IsLoaded; }
        const std::string& GetLoadError() const { return m_LoadError; }
        bool Reload();

        void Start() override;
        void Update(float deltaTime) override;
        void OnDestroy() override;

        nlohmann::json Serialize() const override;
        void Deserialize(const nlohmann::json& json) override;

        const char* GetClassName() const override { return "CSharpScript"; }

    private:
        friend class MonoRuntime;

        std::string m_AssemblyPath =
            "../Assets/Scripts/bin/Debug/netstandard2.0/CreatorGame.dll";
        std::string m_NamespaceName = "CreatorGame";
        std::string m_ClassName = "VerticalBob";
        std::string m_LoadError;
        bool m_IsLoaded = false;
    };
}
