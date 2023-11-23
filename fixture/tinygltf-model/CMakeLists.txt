set(TINYGLTF_MODEL "tinygltf-model")

add_executable(
  ${TINYGLTF_MODEL}

  # Build implementation along with our executable
  ${PROJECT_SOURCE_DIR}/include/GLAD/glad.c

  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_include_directories(
  ${TINYGLTF_MODEL}
  PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

# tinygltf is available by default due to nature of cmake and vcpkg integration

target_link_libraries(
  ${TINYGLTF_MODEL}
  PRIVATE
  glfw
  OpenGL::GL
)
