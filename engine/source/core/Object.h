#pragma once

#include <string>

namespace eng
{
    // UE 式基类：提供运行时类型信息，禁止拷贝/移动
    class Object
    {
    public:
        Object() = default;
        virtual ~Object() = default;

        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;
        Object(Object&&) = delete;
        Object& operator=(Object&&) = delete;

        // 子类重写此方法返回类名
        virtual const char* GetClassName() const = 0;

        // 运行时类型检查
        template<typename T>
        bool IsA() const
        {
            return dynamic_cast<const T*>(this) != nullptr;
        }
    };
}