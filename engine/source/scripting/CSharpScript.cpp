#include "scripting/CSharpScript.h"
#include "scripting/MonoRuntime.h"

namespace eng
{
    void CSharpScript::Start()
    {
        Reload();
    }

    void CSharpScript::Update(float deltaTime)
    {
        if (m_IsLoaded && !MonoRuntime::GetInstance().InvokeUpdate(*this, deltaTime))
        {
            m_LoadError = MonoRuntime::GetInstance().GetLastError();
        }
    }

    void CSharpScript::OnDestroy()
    {
        MonoRuntime::GetInstance().DestroyScriptInstance(*this);
        m_IsLoaded = false;
    }

    bool CSharpScript::Reload()
    {
        MonoRuntime& runtime = MonoRuntime::GetInstance();
        runtime.DestroyScriptInstance(*this);
        m_IsLoaded = false;
        m_LoadError.clear();

        if (!runtime.Initialize())
        {
            m_LoadError = runtime.GetLastError();
            return false;
        }

        m_IsLoaded = runtime.CreateScriptInstance(*this);
        if (!m_IsLoaded)
        {
            m_LoadError = runtime.GetLastError();
        }
        return m_IsLoaded;
    }

    nlohmann::json CSharpScript::Serialize() const
    {
        return {
            {"assembly", m_AssemblyPath},
            {"namespace", m_NamespaceName},
            {"class", m_ClassName}
        };
    }

    void CSharpScript::Deserialize(const nlohmann::json& json)
    {
        m_AssemblyPath = json.value("assembly", m_AssemblyPath);
        m_NamespaceName = json.value("namespace", m_NamespaceName);
        m_ClassName = json.value("class", m_ClassName);
    }
}
