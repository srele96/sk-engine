#include <iostream>
#include <vector>

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return EXIT_FAILURE;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow *window =
      glfwCreateWindow(800, 600, "Triangle 3D", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cerr << "Failed to initialize GLAD\n";
    glfwTerminate();
    return EXIT_FAILURE;
  }

  const GLchar *vertex_shader_source{R"glsl(
    #version 430 core
    layout (location = 0) in vec3 aPos;

    uniform mat4 matrix;

    void main() {
      gl_Position = matrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    }
  )glsl"};
  const GLuint vertex_shader{glCreateShader(GL_VERTEX_SHADER)};
  glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
  glCompileShader(vertex_shader);

  const GLchar *fragment_shader_source{R"glsl(
    #version 430 core
    out vec4 FragColor;
    void main() {
      FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
  )glsl"};
  const GLuint fragment_shader{glCreateShader(GL_FRAGMENT_SHADER)};
  glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
  glCompileShader(fragment_shader);

  const GLuint shader_program{glCreateProgram()};
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  std::vector<GLfloat> pyramid{// top
                               0.0f, 0.5f, 0.0f, 1.0f,
                               // left bottom backward
                               -0.5f, -0.5f, 0.5f, 1.0f,
                               // right bottom backward
                               0.5f, -0.5f, 0.5f, 1.0f,
                               // down forward
                               0.0f, -0.5f, -0.5f, 1.0f};
  std::vector<GLuint> indices{// front
                              0, 1, 2,
                              // left
                              0, 1, 3,
                              // right
                              0, 2, 3,
                              // bottom
                              1, 2, 3};

  GLuint vao;
  glGenVertexArrays(1, &vao);
  // We have to bind before we associate vertex attributes with vertex array
  // object.
  glBindVertexArray(vao);

  // https://www.khronos.org/opengl/wiki/Buffer_Object
  // Observe the following:
  // We only store unformatted memory data inside a buffer which we simply call
  // vertex buffer object or VBO. That indicates we are storing vertex data,
  // however, the OpenGL doesn't know what are we storing. I assume to answer
  // that question, we have to provide information on interpretation of the data
  // sent to it. That's where glVertexAttribPointer comes in. It tells OpenGL to
  // associate with current vertex array object (vao) with the buffer object. It
  // tells the vertex array object how to interpret the vertex buffer object
  // data.
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, pyramid.size() * sizeof(GLfloat),
               pyramid.data(), GL_STATIC_DRAW);

  // This function specifies the details of how the vertex attribute data is
  // organized within the vertex buffer. It specifies the index of the generic
  // vertex attribute (in this case, 0), the number of components per attribute,
  // the data type, whether the data should be normalized, the stride (byte
  // offset) between consecutive attributes, and the pointer to the first
  // component of the first attribute. This function associates the specified
  // vertex attribute with the currently bound VBO.
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
  // We have to enable only after we associate vertex data with vertex array
  // object.
  //
  // We may have multiple vertex array objects and switch between them. Some of
  // them can be enabled or disabled.
  //
  // This function enables the generic vertex
  // attribute at the specified index and tells OpenGL which vao to use for
  // drawing.
  glEnableVertexAttribArray(0);

  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
               indices.data(), GL_STATIC_DRAW);

  // --------------------------------------------------------------------------
  // Binding another vertex array object will make the OpenGL use it, however it
  // does not have any vertex attribute associated with it.

  // GLuint empty_vao;
  // glGenVertexArrays(1, &empty_vao);
  // glBindVertexArray(empty_vao);
  // --------------------------------------------------------------------------

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader_program);

    const float degrees_x{0.0f};
    const float degrees_y{0.0f};
    const float degrees_z{0.0f};

    glm::mat4 identity{glm::mat4(1.0f)};

    glm::mat4 x_rotation_matrix{glm::rotate(identity, glm::radians(degrees_x),
                                            glm::vec3(1.0f, 0.0f, 0.0f))};
    glm::mat4 y_rotation_matrix{glm::rotate(identity, glm::radians(degrees_y),
                                            glm::vec3(0.0f, 1.0f, 0.0f))};
    glm::mat4 z_rotation_matrix{glm::rotate(identity, glm::radians(degrees_z),
                                            glm::vec3(0.0f, 0.0f, 1.0f))};
    glm::mat4 matrix{x_rotation_matrix * y_rotation_matrix * z_rotation_matrix};

    GLint matrix_location{glGetUniformLocation(shader_program, "matrix")};
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(matrix));

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

    glfwSwapBuffers(window);
  }

  glDeleteProgram(shader_program);
  glfwTerminate();

  return EXIT_SUCCESS;
}
