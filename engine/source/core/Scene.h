#pragma once

#include "GameObject.h"
#include <string>
#include <vector>
#include <memory>

namespace eng
{
    class PhysicsWorld;

    class Scene
    {
    public:
        explicit Scene(const std::string& name = "Scene");
        ~Scene();

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& name) { m_Name = name; }

        // 创建根 GameObject
        GameObject* CreateGameObject(const std::string& name = "GameObject");

        // 销毁 GameObject
        void DestroyGameObject(GameObject* go);

        // 添加已存在的 GameObject（用于反序列化）
        void AddGameObject(std::unique_ptr<GameObject> go);

        // 清空所有根对象
        void Clear();

        // 获取所有根对象
        const std::vector<std::unique_ptr<GameObject>>& GetRootGameObjects() const { return m_RootGameObjects; }

        // 更新所有 GameObject
        void Update(float deltaTime);

        // 物理世界
        PhysicsWorld* GetPhysicsWorld() const { return m_PhysicsWorld.get(); }

    private:
        std::string m_Name;
        std::vector<std::unique_ptr<GameObject>> m_RootGameObjects;
        std::unique_ptr<PhysicsWorld> m_PhysicsWorld;
    };
}