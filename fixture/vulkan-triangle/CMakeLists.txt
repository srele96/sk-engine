add_executable(
  vulkan-triangle

  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Avoid using glob to find files
# https://news.ycombinator.com/item?id=14423481
set(
  SPV_FILES

  # Error prone, the whole problem and solution for these is problematic
  # i have to embed the name of these in the executable AND here
  # they are also compiled which is also problematic because they can change
  # the name
  ${CMAKE_CURRENT_SOURCE_DIR}/shader.vert.spv
  ${CMAKE_CURRENT_SOURCE_DIR}/shader.frag.spv
)

# copy files to make them available at runtime for the executable as relative
# path to it
foreach(SPV_FILE ${SPV_FILES})
  configure_file(${SPV_FILE} ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
endforeach(SPV_FILE ${SPV_FILES})

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
