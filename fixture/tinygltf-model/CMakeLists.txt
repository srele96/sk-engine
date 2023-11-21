cmake_minimum_required(VERSION 3.20)

set(TINYGLTF_MODEL "tinygltf-model")

project(
  ${TINYGLTF_MODEL}
  VERSION 0.1.0
  LANGUAGES CXX
)

add_executable(
  ${TINYGLTF_MODEL}
  main.cpp
)

# tinygltf is available by default due to nature of cmake and vcpkg integration

target_link_libraries(
  ${TINYGLTF_MODEL}
  PRIVATE
  glfw
  OpenGL::GL
)
