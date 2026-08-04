cmake_minimum_required(VERSION 3.16)
project(Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC
    source/Application.cpp
    source/CreatorEngine.cpp
    source/input/InputManager.cpp
    source/graphics/shaderprogram.cpp
    source/Application.h
    source/CreatorEngine.h
    source/input/InputManager.h
    source/graphics/shaderprogram.h
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source)

if(APPLE)
    find_package(glfw3 REQUIRED)
    find_package(GLEW REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC glfw GLEW::glew)
elseif(WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/include)
endif()