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
        // Components unregister themselves from PhysicsWorld during destruction.
        // Keep the world alive until every GameObject has been released.
        m_GameObjects.clear();
        m_PhysicsWorld.reset();
    }

    GameObject* Scene::CreateGameObject(const std::string& name)
    {
        auto go = std::make_unique<GameObject>(name);
        go->SetScene(this);
        GameObject* ptr = go.get();
        m_GameObjects.push_back(std::move(go));
        return ptr;
    }

    void Scene::DestroyGameObject(GameObject* go)
    {
        if (!go || !Contains(go))
        {
            return;
        }

        std::vector<GameObject*> children;
        for (Transform* child : go->GetChildren())
        {
            if (child && child->GetGameObject())
            {
                children.push_back(child->GetGameObject());
            }
        }
        for (GameObject* child : children)
        {
            DestroyGameObject(child);
        }

        go->SetParent(nullptr, false);
        auto it = std::find_if(m_GameObjects.begin(), m_GameObjects.end(),
            [go](const std::unique_ptr<GameObject>& item) { return item.get() == go; });
        if (it != m_GameObjects.end())
        {
            m_GameObjects.erase(it);
        }
    }

    void Scene::AddGameObject(std::unique_ptr<GameObject> go)
    {
        go->SetScene(this);
        m_GameObjects.push_back(std::move(go));
    }

    void Scene::Clear()
    {
        m_GameObjects.clear();
    }

    std::vector<GameObject*> Scene::GetRootGameObjects() const
    {
        std::vector<GameObject*> roots;
        roots.reserve(m_GameObjects.size());
        for (const auto& go : m_GameObjects)
        {
            if (go && !go->GetParent())
            {
                roots.push_back(go.get());
            }
        }
        return roots;
    }

    bool Scene::Contains(const GameObject* go) const
    {
        return std::any_of(m_GameObjects.begin(), m_GameObjects.end(),
            [go](const std::unique_ptr<GameObject>& item) {
                return item.get() == go;
            });
    }

    void Scene::Update(float deltaTime)
    {
        // 物理模拟
        if (m_PhysicsWorld)
        {
            m_PhysicsWorld->Step(deltaTime);
        }

        // 更新所有 GameObject
        for (auto& go : m_GameObjects)
        {
            go->Update(deltaTime);
        }
    }
}
