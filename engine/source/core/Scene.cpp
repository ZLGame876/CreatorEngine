#include "core/Scene.h"
#include "physics/PhysicsWorld.h"
#include <algorithm>

namespace eng
{
    Scene::Scene(const std::string& name)
        : m_Name(name)
        , m_PhysicsWorld(std::make_unique<PhysicsWorld>())
    {
    }

    Scene::~Scene()
    {
    }

    GameObject* Scene::CreateGameObject(const std::string& name)
    {
        auto go = std::make_unique<GameObject>(name);
        go->SetScene(this);
        GameObject* ptr = go.get();
        m_RootGameObjects.push_back(std::move(go));
        return ptr;
    }

    void Scene::DestroyGameObject(GameObject* go)
    {
        auto it = std::find_if(m_RootGameObjects.begin(), m_RootGameObjects.end(),
            [go](const std::unique_ptr<GameObject>& item) { return item.get() == go; });
        if (it != m_RootGameObjects.end())
        {
            m_RootGameObjects.erase(it);
        }
    }

    void Scene::AddGameObject(std::unique_ptr<GameObject> go)
    {
        go->SetScene(this);
        m_RootGameObjects.push_back(std::move(go));
    }

    void Scene::Clear()
    {
        m_RootGameObjects.clear();
    }

    void Scene::Update(float deltaTime)
    {
        // 物理模拟
        if (m_PhysicsWorld)
        {
            m_PhysicsWorld->Step(deltaTime);
        }

        // 更新所有 GameObject
        for (auto& go : m_RootGameObjects)
        {
            go->Update(deltaTime);
        }
    }
}