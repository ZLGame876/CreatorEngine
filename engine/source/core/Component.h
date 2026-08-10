#pragma once

#include "Object.h"
#include <nlohmann/json.hpp>

namespace eng
{
    class GameObject;

    // 组件基类：挂载到 GameObject 上
    class Component : public Object
    {
        friend class GameObject;

    public:
        Component() = default;
        virtual ~Component() = default;

        // 生命周期回调
        virtual void Awake()  {}
        virtual void Start()  {}
        virtual void Update(float deltaTime) {}
        virtual void OnDestroy() {}

        // 序列化接口
        virtual nlohmann::json Serialize() const { return {}; }
        virtual void Deserialize(const nlohmann::json& json) {}

        // 获取所属 GameObject
        GameObject* GetGameObject() const { return m_GameObject; }

        // 获取 Transform 快捷访问
        class Transform* GetTransform() const;

        const char* GetClassName() const override { return "Component"; }

    private:
        GameObject* m_GameObject = nullptr;
        bool m_HasStarted = false;
    };
}
