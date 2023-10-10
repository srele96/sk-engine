# Avoid using glob to find files
# https://news.ycombinator.com/item?id=14423481
set(
  SHADERS

  ${CMAKE_CURRENT_SOURCE_DIR}/shader.vert
  ${CMAKE_CURRENT_SOURCE_DIR}/shader.frag
)

# Compile shaders to SPIR-V format.
foreach(SHADER ${SHADERS})
  add_custom_command(
    OUTPUT ${SHADER}.spv
    COMMAND glslc ${SHADER} -o ${SHADER}.spv
    DEPENDS ${SHADER}
  )
  # Provide dependency list.
  list(APPEND SHADERS_SPV ${SHADER}.spv)
endforeach()

# Create dependency target so the executable can depend on compiled shaders.
add_custom_target(
  compile-shaders
  DEPENDS
  ${SHADERS_SPV}
)

# Copy shaders to the executable directory.
# Make them available at runtime by relative path.
foreach(SHADER_SPV ${SHADERS_SPV})
  add_custom_command(
    TARGET compile-shaders POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_SPV} ${CMAKE_CURRENT_BINARY_DIR}
  )
endforeach()

add_executable(
  vulkan-triangle

  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/explore.h
  ${CMAKE_CURRENT_SOURCE_DIR}/explore.cpp
)

# Trigger the compilation of shaders.
add_dependencies(
  vulkan-triangle
  compile-shaders
)

target_link_libraries(
  vulkan-triangle
  PRIVATE
  glfw
  Vulkan::Vulkan
  GPUOpen::VulkanMemoryAllocator
  # no idea how to link sdl2 to display graphical window... ill use glfw for
  # now because i can save some time cus its already set up
  # 
  # additionaly they mentioned usage of imgui for debugging purposes...
)
