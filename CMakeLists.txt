cmake_minimum_required(VERSION 3.16)

project(SpaceInvaders LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.9
)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)

set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
set(GLAD_API "gl=3.3" CACHE STRING "" FORCE)

FetchContent_MakeAvailable(glfw glad)

find_package(OpenGL REQUIRED)

add_executable(SpaceInvaders
    src/main.cpp
    src/Game.cpp
    src/Shader.cpp
)

target_include_directories(SpaceInvaders PRIVATE src)
target_compile_definitions(SpaceInvaders PRIVATE SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")

target_link_libraries(SpaceInvaders PRIVATE glad glfw OpenGL::GL)

if(MSVC)
    target_compile_options(SpaceInvaders PRIVATE /W4)
else()
    target_compile_options(SpaceInvaders PRIVATE -Wall -Wextra -Wpedantic)
endif()