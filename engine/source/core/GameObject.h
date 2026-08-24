#pragma once

#include "core/Object.h"
#include "core/Transform.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include <stdexcept>

namespace eng
{
    class Component;
    class Scene;

    class GameObject : public Object
    {
    public:
        explicit GameObject(const std::string& name = "GameObject");
        ~GameObject() override;

        // 场景归属
        void SetScene(Scene* scene) { m_Scene = scene; }
        Scene* GetScene() const { return m_Scene; }

        // 名称
        void SetName(const std::string& name) { m_Name = name; }
        const std::string& GetName() const { return m_Name; }

        // 激活状态
        void SetActive(bool active);
        bool IsActive() const { return m_ActiveSelf && (m_Transform->GetParent() ? m_Transform->GetParent()->GetGameObject()->IsActive() : true); }
        bool IsActiveSelf() const { return m_ActiveSelf; }

        // Transform（始终存在）
        Transform* GetTransform() const { return m_Transform; }

        // 组件管理
        template<typename T, typename... Args>
        T* AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
            auto comp = std::make_unique<T>(std::forward<Args>(args)...);
            T* ptr = comp.get();
            comp->m_GameObject = this;
            m_Components.push_back(std::move(comp));
            ptr->Awake();
            return ptr;
        }

        template<typename T>
        T* GetComponent() const
        {
            static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
            for (auto& comp : m_Components)
            {
                if (comp->IsA<T>())
                {
                    return static_cast<T*>(comp.get());
                }
            }
            return nullptr;
        }

        template<typename T>
        std::vector<T*> GetComponents() const
        {
            static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
            std::vector<T*> result;
            for (auto& comp : m_Components)
            {
                if (comp->IsA<T>())
                {
                    result.push_back(static_cast<T*>(comp.get()));
                }
            }
            return result;
        }

        // 获取所有组件（非模板版本，用于编辑器）
        std::vector<Component*> GetAllComponents() const
        {
            std::vector<Component*> result;
            for (auto& comp : m_Components)
            {
                result.push_back(comp.get());
            }
            return result;
        }

        // 层级快捷方法
        GameObject* GetParent() const;
        bool SetParent(GameObject* parent, bool worldPositionStays = true);
        const std::vector<Transform*>& GetChildren() const { return m_Transform->GetChildren(); }

        // Transform 不能被移除。
        bool RemoveComponent(Component* component);

        // 更新所有组件
        void Update(float deltaTime);

        // 序列化
        nlohmann::json Serialize() const;
        static GameObject* Deserialize(Scene& scene, const nlohmann::json& json,
                                       GameObject* parent = nullptr);

        const char* GetClassName() const override { return "GameObject"; }

    private:
        std::string m_Name;
        bool m_ActiveSelf = true;
        Transform* m_Transform = nullptr;  // 生命周期由 m_Components 管理
        std::vector<std::unique_ptr<Component>> m_Components;
        Scene* m_Scene = nullptr;  // 场景归属
    };
}
