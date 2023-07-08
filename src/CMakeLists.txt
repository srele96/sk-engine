add_executable(
  render-engine

  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/glad.c
)

target_include_directories(
  render-engine
  PRIVATE
  # Not sure if adding path like this is great practice of CMake.
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
  render-engine
  PRIVATE
  glfw
  OpenGL::GL
)
