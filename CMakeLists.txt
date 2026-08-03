cmake_minimum_required(VERSION 3.16)


project(GameDevelopment)

set (PROJECT_SOURCE_FILES
    Source/Main.cpp
)

//添加应用
include_directories(Source)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})

add_subdirectory(Engine "${CMAKE_CURRENT_BINARY_DIR}/Engine_build")

//链接引擎库
target_link_libraries(${PROJECT_NAME} 
    Engine
)