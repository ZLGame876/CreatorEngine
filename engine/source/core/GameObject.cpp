#include "core/GameObject.h"
#include "core/Script.h"
#include "graphics/SpriteRenderer.h"
#include "graphics/Camera.h"
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

    void GameObject::Update(float deltaTime)
    {
        if (!IsActive()) return;

        for (auto& comp : m_Components)
        {
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

        return json;
    }

    std::unique_ptr<GameObject> GameObject::Deserialize(const nlohmann::json& json)
    {
        std::string name = json.value("name", "GameObject");
        auto go = std::make_unique<GameObject>(name);
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
                        auto& data = compJson["data"];
                        if (data.contains("color"))
                            sr->SetColor(data["color"][0].get<float>(), data["color"][1].get<float>(),
                                        data["color"][2].get<float>(), data["color"][3].get<float>());
                        if (data.contains("size"))
                            sr->SetSize(data["size"][0].get<float>(), data["size"][1].get<float>());
                        if (data.contains("layer"))
                            sr->SetLayer(data["layer"].get<int>());
                        if (data.contains("orderInLayer"))
                            sr->SetOrderInLayer(data["orderInLayer"].get<int>());
                    }
                }
                else if (type == "Camera")
                {
                    auto* cam = go->AddComponent<Camera>();
                    if (compJson.contains("data"))
                    {
                        auto& data = compJson["data"];
                        if (data.contains("projectionType"))
                        {
                            if (data["projectionType"].get<int>() == 0)
                                cam->SetOrthographic(data.value("orthoSize", 5.0f));
                            else
                                cam->SetPerspective(data.value("fov", 60.0f));
                        }
                    }
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
                            script->m_GameObject = go.get();
                            go->m_Components.push_back(std::unique_ptr<Component>(script.release()));
                        }
                    }
                }
            }
        }

        return go;
    }
}