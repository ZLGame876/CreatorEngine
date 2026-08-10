#include "core/GameObject.h"
#include "core/Scene.h"
#include "core/Script.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Camera.h"
#include "physics/Rigidbody2D.h"
#include "physics/BoxCollider2D.h"
#include "physics/CircleCollider2D.h"
#include "scripting/CSharpScript.h"
#include <algorithm>
#include <iostream>

namespace eng
{
    GameObject::GameObject(const std::string& name)
        : m_Name(name)
    {
        // Transform 是内置必选组件，手动构造以绕过 AddComponent 的 Awake 调用
        auto trans = std::make_unique<Transform>();
        m_Transform = trans.get();
        m_Transform->m_GameObject = this;
        m_Components.push_back(std::move(trans));
    }

    GameObject::~GameObject()
    {
        for (auto& comp : m_Components)
        {
            comp->OnDestroy();
        }
    }

    void GameObject::SetActive(bool active)
    {
        m_ActiveSelf = active;
    }

    GameObject* GameObject::GetParent() const
    {
        Transform* parentTrans = m_Transform->GetParent();
        return parentTrans ? parentTrans->GetGameObject() : nullptr;
    }

    bool GameObject::SetParent(GameObject* parent, bool worldPositionStays)
    {
        if (parent && parent->GetScene() != m_Scene)
        {
            return false;
        }
        return m_Transform->SetParent(parent ? parent->GetTransform() : nullptr,
                                      worldPositionStays);
    }

    bool GameObject::RemoveComponent(Component* component)
    {
        if (!component || component == m_Transform)
        {
            return false;
        }

        const auto it = std::find_if(m_Components.begin(), m_Components.end(),
            [component](const std::unique_ptr<Component>& item) {
                return item.get() == component;
            });
        if (it == m_Components.end())
        {
            return false;
        }

        (*it)->OnDestroy();
        m_Components.erase(it);
        return true;
    }

    void GameObject::Update(float deltaTime)
    {
        if (!IsActive()) return;

        for (auto& comp : m_Components)
        {
            if (!comp->m_HasStarted)
            {
                comp->Start();
                comp->m_HasStarted = true;
            }
            comp->Update(deltaTime);
        }
    }

    nlohmann::json GameObject::Serialize() const
    {
        nlohmann::json json;
        json["name"] = m_Name;
        json["active"] = m_ActiveSelf;

        // 序列化 Transform
        if (m_Transform)
        {
            json["transform"] = {
                {"position", {m_Transform->GetPosition().x, m_Transform->GetPosition().y, m_Transform->GetPosition().z}},
                {"rotation", {m_Transform->GetEulerAngles().x, m_Transform->GetEulerAngles().y, m_Transform->GetEulerAngles().z}},
                {"scale", {m_Transform->GetScale().x, m_Transform->GetScale().y, m_Transform->GetScale().z}}
            };
        }

        // 序列化其他组件
        json["components"] = nlohmann::json::array();
        for (auto& comp : m_Components)
        {
            // Transform 已经在上面单独处理
            if (comp.get() == m_Transform) continue;

            nlohmann::json compJson;
            compJson["type"] = comp->GetClassName();
            nlohmann::json data = comp->Serialize();
            if (!data.empty())
            {
                compJson["data"] = data;
            }
            json["components"].push_back(compJson);
        }

        json["children"] = nlohmann::json::array();
        for (Transform* child : m_Transform->GetChildren())
        {
            if (child && child->GetGameObject())
            {
                json["children"].push_back(child->GetGameObject()->Serialize());
            }
        }

        return json;
    }

    GameObject* GameObject::Deserialize(Scene& scene, const nlohmann::json& json,
                                        GameObject* parent)
    {
        std::string name = json.value("name", "GameObject");
        GameObject* go = scene.CreateGameObject(name);
        if (!go)
        {
            return nullptr;
        }

        if (parent && !go->SetParent(parent, false))
        {
            scene.DestroyGameObject(go);
            return nullptr;
        }

        go->SetActive(json.value("active", true));

        // 反序列化 Transform
        if (json.contains("transform"))
        {
            auto& t = json["transform"];
            auto pos = t["position"];
            auto rot = t["rotation"];
            auto scl = t["scale"];
            go->GetTransform()->SetPosition(pos[0].get<float>(), pos[1].get<float>(), pos[2].get<float>());
            go->GetTransform()->SetEulerAngles(glm::vec3(rot[0].get<float>(), rot[1].get<float>(), rot[2].get<float>()));
            go->GetTransform()->SetScale(scl[0].get<float>(), scl[1].get<float>(), scl[2].get<float>());
        }

        // 反序列化其他组件
        if (json.contains("components"))
        {
            for (auto& compJson : json["components"])
            {
                std::string type = compJson.value("type", "");
                if (type == "SpriteRenderer")
                {
                    auto* sr = go->AddComponent<SpriteRenderer>();
                    if (compJson.contains("data"))
                    {
                        sr->Deserialize(compJson["data"]);
                    }
                }
                else if (type == "Camera")
                {
                    auto* cam = go->AddComponent<Camera>();
                    if (compJson.contains("data"))
                    {
                        cam->Deserialize(compJson["data"]);
                    }
                }
                else if (type == "Rigidbody2D")
                {
                    auto* component = go->AddComponent<Rigidbody2D>();
                    if (compJson.contains("data")) component->Deserialize(compJson["data"]);
                }
                else if (type == "BoxCollider2D")
                {
                    auto* component = go->AddComponent<BoxCollider2D>();
                    if (compJson.contains("data")) component->Deserialize(compJson["data"]);
                }
                else if (type == "CircleCollider2D")
                {
                    auto* component = go->AddComponent<CircleCollider2D>();
                    if (compJson.contains("data")) component->Deserialize(compJson["data"]);
                }
                else if (type == "CSharpScript")
                {
                    auto* component = go->AddComponent<CSharpScript>();
                    if (compJson.contains("data")) component->Deserialize(compJson["data"]);
                }
                else if (type == "Script")
                {
                    if (compJson.contains("scriptName"))
                    {
                        std::string scriptName = compJson["scriptName"].get<std::string>();
                        auto script = Script::CreateScript(scriptName);
                        if (script)
                        {
                            if (compJson.contains("data"))
                            {
                                script->Deserialize(compJson["data"]);
                            }
                            // 手动添加脚本组件（绕过 AddComponent 的 Awake）
                            script->m_GameObject = go;
                            Script* scriptPtr = script.get();
                            go->m_Components.push_back(std::unique_ptr<Component>(script.release()));
                            scriptPtr->Awake();
                        }
                    }
                }
            }
        }

        if (json.contains("children") && json["children"].is_array())
        {
            for (const auto& childJson : json["children"])
            {
                Deserialize(scene, childJson, go);
            }
        }

        return go;
    }
}
