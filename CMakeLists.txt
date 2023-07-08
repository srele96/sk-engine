cmake_minimum_required(VERSION 3.14)

project(render-engine)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(src)
