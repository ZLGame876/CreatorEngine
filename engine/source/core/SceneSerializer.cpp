#include "core/SceneSerializer.h"
#include "core/Scene.h"
#include "core/GameObject.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

namespace eng
{
    bool SceneSerializer::Save(Scene* scene, const std::string& filePath)
    {
        if (!scene) return false;

        nlohmann::json root;
        root["name"] = scene->GetName();
        root["objects"] = nlohmann::json::array();

        for (const auto& goPtr : scene->GetRootGameObjects())
        {
            if (goPtr)
            {
                root["objects"].push_back(goPtr->Serialize());
            }
        }

        std::ofstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "无法打开文件: " << filePath << std::endl;
            return false;
        }

        file << root.dump(4);
        file.close();

        std::cout << "场景已保存: " << filePath << std::endl;
        return true;
    }

    bool SceneSerializer::Load(Scene* scene, const std::string& filePath)
    {
        if (!scene) return false;

        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "无法打开文件: " << filePath << std::endl;
            return false;
        }

        nlohmann::json root;
        try
        {
            file >> root;
        }
        catch (const nlohmann::json::parse_error& e)
        {
            std::cerr << "JSON 解析错误: " << e.what() << std::endl;
            return false;
        }

        // 清空当前场景
        scene->Clear();

        // 加载场景名
        if (root.contains("name"))
        {
            scene->SetName(root["name"].get<std::string>());
        }

        // 加载 GameObject
        if (root.contains("objects"))
        {
            for (auto& objJson : root["objects"])
            {
                auto go = GameObject::Deserialize(objJson);
                if (go)
                {
                    scene->AddGameObject(std::move(go));
                }
            }
        }

        std::cout << "场景已加载: " << filePath << std::endl;
        return true;
    }
}
