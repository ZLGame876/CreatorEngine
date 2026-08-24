#pragma once

#include "core/Component.h"
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>

namespace eng
{
    class Script : public Component
    {
    public:
        Script() = default;
        ~Script() override = default;

        // 序列化接口
        virtual nlohmann::json Serialize() const { return {}; }
        virtual void Deserialize(const nlohmann::json& json) {}

        // 工厂注册
        using FactoryFunc = std::function<std::unique_ptr<Script>()>;

        static void RegisterScript(const std::string& name, FactoryFunc factory)
        {
            GetRegistry()[name] = factory;
        }

        static std::unique_ptr<Script> CreateScript(const std::string& name)
        {
            auto& registry = GetRegistry();
            auto it = registry.find(name);
            if (it != registry.end())
            {
                return it->second();
            }
            return nullptr;
        }

        static std::unordered_map<std::string, FactoryFunc>& GetRegistry()
        {
            static std::unordered_map<std::string, FactoryFunc> s_Registry;
            return s_Registry;
        }

        static const std::string& GetStaticClassName()
        {
            static const std::string name = "Script";
            return name;
        }

        const char* GetClassName() const override { return "Script"; }

    private:
        // 自动注册辅助类
        template<typename T>
        struct AutoRegister
        {
            AutoRegister(const std::string& name)
            {
                RegisterScript(name, []() -> std::unique_ptr<Script> {
                    return std::make_unique<T>();
                });
            }
        };
    };

    // 注册宏
    #define REGISTER_SCRIPT(ClassName, ScriptName) \
        static eng::Script::AutoRegister<ClassName> s_##ClassName##_Register(ScriptName)
}
