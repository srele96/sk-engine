cmake_minimum_required(VERSION 3.16)

project(gl-model)

# Find the Assimp package installed by Vcpkg
find_package(assimp CONFIG REQUIRED)

# Add the source files for the gl-model target
add_executable(gl-model main.cpp)

# Link with the Assimp library
target_link_libraries(
  gl-model
  PRIVATE
  glfw
  OpenGL::GL
  assimp::assimp
)
