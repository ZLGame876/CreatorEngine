#pragma once

#include <string>

namespace eng
{
    class Scene;

    class SceneSerializer
    {
    public:
        static bool Save(Scene* scene, const std::string& filePath);
        static bool Load(Scene* scene, const std::string& filePath);
    };
}
