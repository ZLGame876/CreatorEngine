cmake_minimum_required(VERSION 3.16)

project(Engine)

set(PROJECT_SOURCE_FILES
    Source/Engine.cpp
    Source/Engine.h
    Source/Application.h
    Source/Application.cpp
)

include_directories(Source)
add_library(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})


//添加GLFW库
add_subdirectory(thirdParty/glfw-3.4 "${CMAKE_CURRENT_BINARY_DIR}/glfw_build")
include_directories(thirdParty/glfw-3.4/include)


//添加Glew库
set(BUILD_UTILS OFF CACHE BOOL "utilities" FORCE)
add_subdirectory(thirdParty/glew/build/CMake "${CMAKE_CURRENT_BINARY_DIR}/glew_build")
include_directories(thirdParty/glew/include)
set_property(TARGET glew PROPERTY FOLDER GLEW)
set_property(TARGET glew_s PROPERTY FOLDER GLEW)

//链接所有第三方库
target_link_libraries(${PROJECT_NAME} 
glfw 
glew_s
)