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
    source/graphics/Framebuffer.cpp
    source/graphics/SpriteRenderer.cpp
    source/graphics/SpriteBatch.cpp
    source/graphics/Camera.cpp
    source/core/Component.cpp
    source/core/Transform.cpp
    source/core/GameObject.cpp
    source/core/Scene.cpp
    source/core/SceneSerializer.cpp
    source/scripting/MonoRuntime.cpp
    source/scripting/CSharpScript.cpp
    source/scripting/NativeHandleRegistry.cpp
    source/editor/Editor.cpp
    source/physics/Rigidbody2D.cpp
    source/physics/PhysicsMaterial.cpp
    source/physics/Collider2D.cpp
    source/physics/BoxCollider2D.cpp
    source/physics/CircleCollider2D.cpp
    source/physics/PhysicsWorld.cpp
    source/gameplay/GameplayComponents.cpp
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
    source/graphics/Framebuffer.h
    source/graphics/SpriteRenderer.h
    source/graphics/SpriteBatch.h
    source/graphics/Camera.h
    source/editor/Editor.h
    source/core/Object.h
    source/core/Component.h
    source/core/ProjectPaths.h
    source/core/Transform.h
    source/core/GameObject.h
    source/core/Scene.h
    source/core/Script.h
    source/core/SceneSerializer.h
    source/scripting/MonoRuntime.h
    source/scripting/CSharpScript.h
    source/scripting/NativeHandleRegistry.h
    source/physics/Rigidbody2D.h
    source/physics/PhysicsMaterial.h
    source/physics/Collider2D.h
    source/physics/BoxCollider2D.h
    source/physics/CircleCollider2D.h
    source/physics/PhysicsWorld.h
    source/gameplay/GameplayComponents.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty
    ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/include
)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/imgui/backends)

if(APPLE)
    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(glm REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw GLEW::glew glm::glm)
elseif(WIN32)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
    endif()
endif()

option(CREATOR_ENABLE_MONO "Enable the embedded Mono C# scripting runtime" OFF)
if(CREATOR_ENABLE_MONO)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(MONO QUIET mono-2)
    endif()

    if(MONO_FOUND)
        target_include_directories(${PROJECT_NAME} PRIVATE ${MONO_INCLUDE_DIRS})
        target_link_directories(${PROJECT_NAME} PUBLIC ${MONO_LIBRARY_DIRS})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${MONO_LINK_LIBRARIES})
    else()
        find_path(MONO_INCLUDE_DIR mono/jit/jit.h
            HINTS $ENV{MONO_ROOT} /Library/Frameworks/Mono.framework/Headers
            PATH_SUFFIXES include/mono-2.0 mono-2.0)
        find_library(MONO_LIBRARY
            NAMES mono-2.0-sgen monosgen-2.0 mono-2.0
            HINTS $ENV{MONO_ROOT} /Library/Frameworks/Mono.framework/Libraries
            PATH_SUFFIXES lib)
        if(NOT MONO_INCLUDE_DIR OR NOT MONO_LIBRARY)
            message(FATAL_ERROR "CREATOR_ENABLE_MONO=ON but the Mono embedding SDK was not found. Set MONO_ROOT or install mono-2 development files.")
        endif()
        target_include_directories(${PROJECT_NAME} PRIVATE ${MONO_INCLUDE_DIR})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${MONO_LIBRARY})
    endif()

    target_compile_definitions(${PROJECT_NAME} PRIVATE CREATOR_ENABLE_MONO=1)

    if(WIN32)
        find_file(MONO_RUNTIME_DLL
            NAMES mono-2.0-sgen.dll monosgen-2.0.dll mono-2.0.dll
            HINTS $ENV{MONO_ROOT}
            PATH_SUFFIXES bin)
        if(NOT MONO_RUNTIME_DLL)
            message(FATAL_ERROR "Mono import library was found but its runtime DLL was not. Set MONO_ROOT to the Mono installation directory.")
        endif()
        set(CREATOR_MONO_RUNTIME_DLL "${MONO_RUNTIME_DLL}" CACHE INTERNAL
            "Mono runtime copied beside CreatorEngine")
    endif()
endif()
