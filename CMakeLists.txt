cmake_minimum_required(VERSION 3.16)
project(CreatorEngine)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

add_executable(${PROJECT_NAME}
    source/main.cpp
    source/Game.cpp
    source/Game.h
)

add_subdirectory(engine "${CMAKE_CURRENT_BINARY_DIR}/Engine_build")

target_include_directories(${PROJECT_NAME} PRIVATE source ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)

if(APPLE)
    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Engine
        glfw
        GLEW::glew
        OpenGL::GL
    )
elseif(WIN32)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
    endif()
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/include)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Engine
        ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/lib/glew32.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/lib/glfw3dll.lib
        opengl32
    )
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/bin/glew32.dll
            ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/bin/glfw3.dll
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    if(CREATOR_ENABLE_MONO AND CREATOR_MONO_RUNTIME_DLL)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CREATOR_MONO_RUNTIME_DLL}
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    endif()
endif()

if(BUILD_TESTING)
    add_executable(CreatorEngineCoreTests
        tests/CoreHierarchyTests.cpp
    )
    target_link_libraries(CreatorEngineCoreTests PRIVATE Engine)

    if(APPLE)
        target_link_libraries(CreatorEngineCoreTests PRIVATE OpenGL::GL)
    elseif(WIN32)
        if(MSVC)
            target_compile_options(CreatorEngineCoreTests PRIVATE /utf-8)
        endif()
        target_link_libraries(CreatorEngineCoreTests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/lib/glew32.lib
            ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/lib/glfw3dll.lib
            opengl32
        )
        add_custom_command(TARGET CreatorEngineCoreTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/bin/glew32.dll
                ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/bin/glfw3.dll
                $<TARGET_FILE_DIR:CreatorEngineCoreTests>
        )
    endif()

    add_test(NAME CoreHierarchy COMMAND CreatorEngineCoreTests)
endif()
