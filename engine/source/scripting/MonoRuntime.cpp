#include "scripting/MonoRuntime.h"

#include "core/GameObject.h"
#include "core/ProjectPaths.h"
#include "core/Transform.h"
#include "scripting/CSharpScript.h"

#include <cstdint>
#include <iostream>
#include <unordered_map>

#if defined(CREATOR_ENABLE_MONO)
    #include <mono/jit/jit.h>
    #include <mono/metadata/assembly.h>
    #include <mono/metadata/debug-helpers.h>
    #include <mono/metadata/mono-config.h>
    #include <mono/metadata/object.h>
#endif

namespace eng
{
#if defined(CREATOR_ENABLE_MONO)
    namespace
    {
        struct ManagedVector3
        {
            float x;
            float y;
            float z;
        };

        void TransformGetPosition(std::uint64_t nativeHandle, ManagedVector3* result)
        {
            if (!nativeHandle || !result) return;
            auto* object = reinterpret_cast<GameObject*>(static_cast<std::uintptr_t>(nativeHandle));
            const glm::vec3 position = object->GetTransform()->GetPosition();
            result->x = position.x;
            result->y = position.y;
            result->z = position.z;
        }

        void TransformSetPosition(std::uint64_t nativeHandle, const ManagedVector3* value)
        {
            if (!nativeHandle || !value) return;
            auto* object = reinterpret_cast<GameObject*>(static_cast<std::uintptr_t>(nativeHandle));
            object->GetTransform()->SetPosition(value->x, value->y, value->z);
        }

        MonoMethod* FindMethod(MonoClass* klass, const char* name, int parameterCount)
        {
            for (MonoClass* current = klass; current; current = mono_class_get_parent(current))
            {
                if (MonoMethod* method = mono_class_get_method_from_name(current, name, parameterCount))
                {
                    return method;
                }
            }
            return nullptr;
        }

        MonoClassField* FindField(MonoClass* klass, const char* name)
        {
            for (MonoClass* current = klass; current; current = mono_class_get_parent(current))
            {
                if (MonoClassField* field = mono_class_get_field_from_name(current, name))
                {
                    return field;
                }
            }
            return nullptr;
        }
    }
#endif

    struct MonoRuntime::Impl
    {
#if defined(CREATOR_ENABLE_MONO)
        struct ScriptInstance
        {
            std::uint32_t gcHandle = 0;
            MonoMethod* update = nullptr;
            MonoMethod* onDestroy = nullptr;
        };

        MonoDomain* domain = nullptr;
        std::unordered_map<std::string, MonoAssembly*> assemblies;
        std::unordered_map<CSharpScript*, ScriptInstance> instances;
#endif
    };

    MonoRuntime& MonoRuntime::GetInstance()
    {
        static MonoRuntime runtime;
        return runtime;
    }

    MonoRuntime::MonoRuntime()
        : m_Impl(std::make_unique<Impl>())
    {
    }

    MonoRuntime::~MonoRuntime()
    {
        Shutdown();
    }

    bool MonoRuntime::IsAvailable() const
    {
#if defined(CREATOR_ENABLE_MONO)
        return true;
#else
        return false;
#endif
    }

    bool MonoRuntime::IsInitialized() const
    {
#if defined(CREATOR_ENABLE_MONO)
        return m_Impl->domain != nullptr;
#else
        return false;
#endif
    }

    bool MonoRuntime::Initialize(const std::string& domainName)
    {
#if defined(CREATOR_ENABLE_MONO)
        if (m_Impl->domain)
        {
            return true;
        }

        mono_config_parse(nullptr);
        m_Impl->domain = mono_jit_init_version(domainName.c_str(), "v4.0.30319");
        if (!m_Impl->domain)
        {
            m_LastError = "Mono failed to create the root domain";
            return false;
        }

        mono_add_internal_call("CreatorEngine.InternalCalls::Transform_GetPosition",
            reinterpret_cast<const void*>(&TransformGetPosition));
        mono_add_internal_call("CreatorEngine.InternalCalls::Transform_SetPosition",
            reinterpret_cast<const void*>(&TransformSetPosition));
        m_LastError.clear();
        return true;
#else
        (void)domainName;
        m_LastError = "CreatorEngine was built without Mono (enable CREATOR_ENABLE_MONO)";
        return false;
#endif
    }

    void MonoRuntime::Shutdown()
    {
#if defined(CREATOR_ENABLE_MONO)
        if (!m_Impl->domain)
        {
            return;
        }

        for (auto& pair : m_Impl->instances)
        {
            if (pair.second.gcHandle != 0)
            {
                mono_gchandle_free(pair.second.gcHandle);
            }
        }
        m_Impl->instances.clear();
        m_Impl->assemblies.clear();
        MonoDomain* domain = m_Impl->domain;
        m_Impl->domain = nullptr;
        mono_jit_cleanup(domain);
#endif
    }

#if defined(CREATOR_ENABLE_MONO)
    namespace
    {
        bool InvokeManaged(std::uint32_t gcHandle, MonoMethod* method,
                           void** arguments, std::string& error)
        {
            if (!method)
            {
                return true;
            }

            MonoObject* object = mono_gchandle_get_target(gcHandle);
            MonoObject* exception = nullptr;
            mono_runtime_invoke(method, object, arguments, &exception);
            if (!exception)
            {
                return true;
            }

            MonoString* text = mono_object_to_string(exception, nullptr);
            char* utf8 = text ? mono_string_to_utf8(text) : nullptr;
            error = utf8 ? utf8 : "Managed script threw an exception";
            if (utf8) mono_free(utf8);
            return false;
        }
    }
#endif

    bool MonoRuntime::CreateScriptInstance(CSharpScript& script)
    {
#if defined(CREATOR_ENABLE_MONO)
        if (!Initialize())
        {
            return false;
        }

        DestroyScriptInstance(script);
        const std::string assemblyPath =
            ProjectPaths::ResolveResource(script.GetAssemblyPath()).string();
        MonoAssembly* assembly = nullptr;
        const auto assemblyIt = m_Impl->assemblies.find(assemblyPath);
        if (assemblyIt != m_Impl->assemblies.end())
        {
            assembly = assemblyIt->second;
        }
        else
        {
            assembly = mono_domain_assembly_open(m_Impl->domain, assemblyPath.c_str());
            if (!assembly)
            {
                m_LastError = "Mono could not load assembly: " + assemblyPath;
                return false;
            }
            m_Impl->assemblies.emplace(assemblyPath, assembly);
        }

        MonoImage* image = mono_assembly_get_image(assembly);
        MonoClass* klass = mono_class_from_name(image, script.GetNamespaceName().c_str(),
                                                script.GetManagedClassName().c_str());
        if (!klass)
        {
            m_LastError = "Mono could not find class: " + script.GetNamespaceName() + "." +
                          script.GetManagedClassName();
            return false;
        }

        MonoObject* object = mono_object_new(m_Impl->domain, klass);
        if (!object)
        {
            m_LastError = "Mono failed to allocate the managed script";
            return false;
        }
        mono_runtime_object_init(object);

        if (MonoClassField* field = FindField(klass, "NativeHandle"))
        {
            std::uint64_t handle = static_cast<std::uint64_t>(
                reinterpret_cast<std::uintptr_t>(script.GetGameObject()));
            mono_field_set_value(object, field, &handle);
        }

        Impl::ScriptInstance instance;
        instance.gcHandle = mono_gchandle_new(object, false);
        instance.update = FindMethod(klass, "Update", 1);
        instance.onDestroy = FindMethod(klass, "OnDestroy", 0);
        auto inserted = m_Impl->instances.emplace(&script, instance);

        MonoMethod* awake = FindMethod(klass, "Awake", 0);
        MonoMethod* start = FindMethod(klass, "Start", 0);
        if (!InvokeManaged(inserted.first->second.gcHandle, awake, nullptr, m_LastError) ||
            !InvokeManaged(inserted.first->second.gcHandle, start, nullptr, m_LastError))
        {
            DestroyScriptInstance(script);
            return false;
        }

        m_LastError.clear();
        return true;
#else
        (void)script;
        m_LastError = "CreatorEngine was built without Mono (enable CREATOR_ENABLE_MONO)";
        return false;
#endif
    }

    void MonoRuntime::DestroyScriptInstance(CSharpScript& script)
    {
#if defined(CREATOR_ENABLE_MONO)
        const auto it = m_Impl->instances.find(&script);
        if (it == m_Impl->instances.end())
        {
            return;
        }
        InvokeManaged(it->second.gcHandle, it->second.onDestroy, nullptr, m_LastError);
        if (it->second.gcHandle != 0)
        {
            mono_gchandle_free(it->second.gcHandle);
        }
        m_Impl->instances.erase(it);
#else
        (void)script;
#endif
    }

    bool MonoRuntime::InvokeUpdate(CSharpScript& script, float deltaTime)
    {
#if defined(CREATOR_ENABLE_MONO)
        const auto it = m_Impl->instances.find(&script);
        if (it == m_Impl->instances.end())
        {
            m_LastError = "Managed script instance is not loaded";
            return false;
        }
        void* arguments[] = {&deltaTime};
        return InvokeManaged(it->second.gcHandle, it->second.update, arguments, m_LastError);
#else
        (void)script;
        (void)deltaTime;
        m_LastError = "CreatorEngine was built without Mono (enable CREATOR_ENABLE_MONO)";
        return false;
#endif
    }
}
