cmake_minimum_required(VERSION 3.16)
project(Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC
    source/Application.cpp
    source/CreatorEngine.cpp
    source/input/InputManager.cpp
    source/graphics/shaderprogram.cpp
    source/graphics/Texture.cpp
    source/graphics/SpriteRenderer.cpp
    source/graphics/SpriteBatch.cpp
    source/graphics/Camera.cpp
    source/core/Component.cpp
    source/core/Transform.cpp
    source/core/GameObject.cpp
    source/core/Scene.cpp
    source/core/SceneSerializer.cpp
    source/editor/Editor.cpp
    source/physics/Rigidbody2D.cpp
    source/physics/PhysicsMaterial.cpp
    source/physics/Collider2D.cpp
    source/physics/BoxCollider2D.cpp
    source/physics/CircleCollider2D.cpp
    source/physics/PhysicsWorld.cpp
    ../ThirdParty/imgui/imgui.cpp
    ../ThirdParty/imgui/imgui_draw.cpp
    ../ThirdParty/imgui/imgui_tables.cpp
    ../ThirdParty/imgui/imgui_widgets.cpp
    ../ThirdParty/imgui/imgui_demo.cpp
    ../ThirdParty/imgui/backends/imgui_impl_glfw.cpp
    ../ThirdParty/imgui/backends/imgui_impl_opengl3.cpp
    source/Application.h
    source/CreatorEngine.h
    source/input/InputManager.h
    source/graphics/shaderprogram.h
    source/graphics/Texture.h
    source/graphics/SpriteRenderer.h
    source/graphics/SpriteBatch.h
    source/graphics/Camera.h
    source/editor/Editor.h
    source/core/Object.h
    source/core/Component.h
    source/core/Transform.h
    source/core/GameObject.h
    source/core/Scene.h
    source/core/Script.h
    source/core/SceneSerializer.h
    source/physics/Rigidbody2D.h
    source/physics/PhysicsMaterial.h
    source/physics/Collider2D.h
    source/physics/BoxCollider2D.h
    source/physics/CircleCollider2D.h
    source/physics/PhysicsWorld.h
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/imgui/backends)

if(APPLE)
    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glm REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw GLEW::glew glm::glm)
elseif(WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/include)
endif()