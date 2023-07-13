# It's not clear to me on how to reuse CMakeLists.txt code across the project
# and since this is very little code, it's better not to create abstraction.
add_executable(
  triangle-3d

  ${PROJECT_SOURCE_DIR}/include/GLAD/glad.c

  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_include_directories(
  triangle-3d
  PRIVATE
  ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(
  triangle-3d
  PRIVATE
  glfw
  OpenGL::GL
)
