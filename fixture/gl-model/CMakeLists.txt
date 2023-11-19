cmake_minimum_required(VERSION 3.16)

set(GL_MODEL "gl-model")

project(
  ${GL_MODEL}
  VERSION 0.1.0
  LANGUAGES CXX
)

# Find packages installed by Vcpkg
find_package(assimp CONFIG REQUIRED)
find_package(Stb REQUIRED)

# Add the source files for the gl-model target
add_executable(
  ${GL_MODEL}
  main.cpp
)

# Link with the Assimp library
target_link_libraries(
  ${GL_MODEL}
  PRIVATE
  glfw
  OpenGL::GL
  assimp::assimp
)

target_include_directories(
  ${GL_MODEL}
  PRIVATE
  ${STB_INCLUDE_DIRS}
)
