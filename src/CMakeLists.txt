add_executable(
  sk-engine

  ${PROJECT_SOURCE_DIR}/include/GLAD/glad.c

  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_include_directories(
  sk-engine
  PRIVATE
  # Not sure if adding path like this is great practice of CMake.
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
  sk-engine
  PRIVATE
  glfw
  OpenGL::GL
)
