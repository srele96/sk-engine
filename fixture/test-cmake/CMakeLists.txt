cmake_minimum_required(VERSION 3.20)

project(test-cmake)

add_executable(
  test-cmake
  main.cpp
)

option(WORK_WITH_FILES "Use custom commands in the build script" ON)

if(WORK_WITH_FILES)

  message(STATUS "Test add_custom_command on executable target.")

  # --------------------------------------------------------------------
  # Does not recompile or copy the shader if the output is deleted.
  # --------------------------------------------------------------------

  set(SHADER_IN ${CMAKE_CURRENT_SOURCE_DIR}/shader.vert)

  add_custom_command(
    TARGET test-cmake
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling shader."
    COMMAND glslc ${SHADER_IN} -o ${SHADER_IN}.spv
    COMMAND ${CMAKE_COMMAND} -E echo "Copying ${SHADER_IN} to ${CMAKE_CURRENT_BINARY_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_IN} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${SHADER_IN}
  )

else()
  message(STATUS "Test add_custom_command with add_custom_target.")

  set(
    SHADERS
  
    ${CMAKE_CURRENT_SOURCE_DIR}/shader.vert
  )

  foreach(SHADER ${SHADERS})
    add_custom_command(
      OUTPUT ${SHADER}.spv
      COMMAND ${CMAKE_COMMAND} -E echo "Compiling ${SHADER} to ${SHADER}.spv"
      COMMAND glslc ${SHADER} -o ${SHADER}.spv
      DEPENDS ${SHADER}
    )
  
    list(APPEND COMPILED_SHADERS ${SHADER}.spv)
  endforeach()

  add_custom_target(
    compile-shaders ALL
    DEPENDS ${SHADERS}
  )

  # foreach(COMPILED_SHADER ${COMPILED_SHADERS})
  #   add_custom_command(
  #     TARGET compile-shaders
  #     POST_BUILD
  #     COMMAND ${CMAKE_COMMAND} -E echo "Copying ${COMPILED_SHADER} to ${CMAKE_CURRENT_BINARY_DIR}"
  #     COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COMPILED_SHADER} ${CMAKE_CURRENT_BINARY_DIR}
  #     DEPENDS ${COMPILED_SHADER}
  #   )
  # endforeach()

  add_dependencies(
    test-cmake
    compile-shaders
  )

  # each file generates a new file
  # each file is copied
  # is executed every time output file is deleted
  #   - in build directory
  #   - current directory
  # comment runs every time the command runs
  # foreach(FILE ${FILES})
  #   add_custom_command(
  #     TARGET test-cmake
  #     POST_BUILD
  #     COMMAND ${CMAKE_COMMAND} -E copy ${FILE} ${CMAKE_CURRENT_BINARY_DIR}
  #   )
  # endforeach()

endif()
