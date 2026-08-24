#include <core/GameObject.h>
#include <core/ProjectPaths.h>
#include <core/Scene.h>
#include <core/SceneSerializer.h>
#include <scripting/NativeHandleRegistry.h>

#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>

namespace
{
    bool NearlyEqual(const glm::vec3& left, const glm::vec3& right, float epsilon = 0.001f)
    {
        return std::abs(left.x - right.x) <= epsilon &&
               std::abs(left.y - right.y) <= epsilon &&
               std::abs(left.z - right.z) <= epsilon;
    }

    eng::GameObject* FindObject(eng::Scene& scene, const std::string& name)
    {
        for (const auto& object : scene.GetGameObjects())
        {
            if (object && object->GetName() == name)
            {
                return object.get();
            }
        }
        return nullptr;
    }

    bool Require(bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAILED: " << message << std::endl;
        }
        return condition;
    }
}

int main()
{
    if (!Require(std::filesystem::exists(
            eng::ProjectPaths::ResolveResource("source/shaders/grid2d.vert")),
        "resource paths should resolve from nested build directories"))
    {
        return 1;
    }

    eng::Scene scene("Hierarchy Test");
    eng::GameObject* root = scene.CreateGameObject("Root");
    eng::GameObject* child = scene.CreateGameObject("Child");
    eng::GameObject* grandchild = scene.CreateGameObject("Grandchild");

    eng::NativeHandleRegistry& handles = eng::NativeHandleRegistry::GetInstance();
    const eng::NativeHandleRegistry::Handle handle = handles.Acquire(root);
    const eng::NativeHandleRegistry::Handle sharedHandle = handles.Acquire(root);
    if (!Require(handles.Resolve(handle) == root, "native handle should resolve to its object") ||
        !Require(sharedHandle == handle, "same object should share its native handle") ||
        !Require(handles.IsValid(handle), "native handle should be valid while acquired"))
    {
        handles.Release(handle);
        handles.Release(sharedHandle);
        return 1;
    }
    handles.Release(handle);
    if (!Require(handles.IsValid(sharedHandle), "shared native handle should survive one release"))
    {
        handles.Release(sharedHandle);
        return 1;
    }
    handles.Release(sharedHandle);
    if (!Require(!handles.IsValid(handle), "released native handle should be invalid"))
    {
        return 1;
    }

    const eng::NativeHandleRegistry::Handle recycledHandle = handles.Acquire(root);
    if (!Require(recycledHandle != handle,
                 "reused native handle slot should advance its generation"))
    {
        handles.Release(recycledHandle);
        return 1;
    }
    handles.Release(recycledHandle);

    root->GetTransform()->SetPosition(10.0f, 20.0f, 0.0f);
    child->GetTransform()->SetPosition(5.0f, 0.0f, 0.0f);
    grandchild->GetTransform()->SetPosition(0.0f, 3.0f, 0.0f);

    if (!Require(child->SetParent(root, false), "child should accept root parent") ||
        !Require(grandchild->SetParent(child, false), "grandchild should accept child parent") ||
        !Require(scene.GetRootGameObjects().size() == 1, "scene should expose one root") ||
        !Require(!root->SetParent(grandchild), "hierarchy cycle should be rejected"))
    {
        return 1;
    }

    const glm::vec3 childWorldBefore = child->GetTransform()->GetWorldPosition();
    if (!Require(child->SetParent(nullptr, true), "child should detach") ||
        !Require(NearlyEqual(child->GetTransform()->GetWorldPosition(), childWorldBefore),
                 "detaching should preserve world position") ||
        !Require(child->SetParent(root, true), "child should reattach") ||
        !Require(NearlyEqual(child->GetTransform()->GetWorldPosition(), childWorldBefore),
                 "reattaching should preserve world position"))
    {
        return 1;
    }

    root->GetTransform()->SetEulerAngles(glm::vec3(0.0f, 90.0f, 0.0f));
    if (!Require(NearlyEqual(child->GetTransform()->GetForward(),
                             glm::vec3(-1.0f, 0.0f, 0.0f)),
                 "child world direction should inherit parent rotation"))
    {
        return 1;
    }

    const std::filesystem::path scenePath =
        std::filesystem::current_path() / "core-hierarchy-test.scene.json";
    if (!Require(eng::SceneSerializer::Save(&scene, scenePath.string()), "scene should save"))
    {
        return 1;
    }

    eng::Scene loaded;
    if (!Require(eng::SceneSerializer::Load(&loaded, scenePath.string()), "scene should load"))
    {
        std::filesystem::remove(scenePath);
        return 1;
    }
    std::filesystem::remove(scenePath);

    eng::GameObject* loadedRoot = FindObject(loaded, "Root");
    eng::GameObject* loadedChild = FindObject(loaded, "Child");
    eng::GameObject* loadedGrandchild = FindObject(loaded, "Grandchild");
    if (!Require(loaded.GetGameObjects().size() == 3, "three objects should round-trip") ||
        !Require(loaded.GetRootGameObjects().size() == 1, "loaded scene should expose one root") ||
        !Require(loadedChild && loadedChild->GetParent() == loadedRoot,
                 "child parent should round-trip") ||
        !Require(loadedGrandchild && loadedGrandchild->GetParent() == loadedChild,
                 "grandchild parent should round-trip"))
    {
        return 1;
    }

    loaded.DestroyGameObject(loadedRoot);
    if (!Require(loaded.GetGameObjects().empty(), "destroying root should destroy descendants"))
    {
        return 1;
    }

    std::cout << "Core hierarchy tests passed" << std::endl;
    return 0;
}
