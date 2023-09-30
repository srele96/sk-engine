add_executable(
  vulkan-triangle

  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(
  vulkan-triangle
  PRIVATE
  glfw
  Vulkan::Vulkan
  # no idea how to link sdl2 to display graphical window... ill use glfw for
  # now because i can save some time cus its already set up
  # 
  # additionaly they mentioned usage of imgui for debugging purposes...
)
