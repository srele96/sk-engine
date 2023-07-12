#include <functional>
#include <iostream>
#include <vector>

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"

#include "shader.h"

void onSetWindowSize(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

GLuint createShader(const GLenum type, const GLchar *source) {
  GLuint shader{glCreateShader(type)};
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  return shader;
}

GLuint createProgram(const GLchar *vsSource, const GLchar *fsSource) {
  GLuint vertexShader{createShader(GL_VERTEX_SHADER, vsSource)};
  GLuint fragmentShader{createShader(GL_FRAGMENT_SHADER, fsSource)};

  GLuint shaderProgram{glCreateProgram()};
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

namespace comfortExtender {

using EachProgramUniformFunc = std::function<void(
    const GLint size, const GLenum type, const std::string &name)>;

// Try out different approaches to explore what's possible to extend comfort
// zone.
void eachProgramUniform(const GLuint program, EachProgramUniformFunc &func) {
  GLint params;
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &params);

  // According to the documentation, this is the recommended way to retrieve the
  // max length of the uniform variable of a shader because it guarantees that
  // we will have enough space to store the name of the uniform variable.
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glGetActiveUniform.xhtml
  GLint maxLength;
  glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLength);

  for (GLint i{0}; i != params; ++i) {
    GLint size;
    std::vector<GLchar> nameBuffer(maxLength);
    GLenum type;
    glGetActiveUniform(program, i, maxLength, nullptr, &size, &type,
                       nameBuffer.data());
    std::string name{nameBuffer.data()};

    func(size, type, name);
  }
};

} // namespace comfortExtender

int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  constexpr int initialWidth = 800;
  constexpr int initialHeight = 600;
  GLFWwindow *window{glfwCreateWindow(
      initialWidth, initialHeight, "GLFW Glad Test Window", nullptr, nullptr)};
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

  int width;
  int height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);

  glfwSetWindowSizeCallback(window, onSetWindowSize);

  std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";

  GLuint shaderProgram1{createProgram(shader::vertex::translateByOffset.c_str(),
                                      shader::fragment::red.c_str())};
  GLuint shaderProgram2{createProgram(shader::vertex::translateByOffset.c_str(),
                                      shader::fragment::blue.c_str())};

  comfortExtender::EachProgramUniformFunc print{
      [](const GLint size, const GLenum type, const std::string &name) {
        std::cout << "shaderProgram1:\n"
                  << "Name: \"" << name << "\", "
                  << "Size: \"" << size << "\", "
                  << "Type: \"" << type << "\"\n";
      }};
  comfortExtender::eachProgramUniform(shaderProgram1, print);

  const std::vector<GLfloat> triangle{-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                                      0.0f,  0.0f,  0.5f, 0.0f};
  const std::vector<GLfloat> square{
      -0.5f, -0.5f, 0.0f, // Bottom-left vertex of the square
      0.5f,  -0.5f, 0.0f, // Bottom-right vertex of the square
      -0.5f, 0.5f,  0.0f, // Top-left vertex of the square
      0.5f,  0.5f,  0.0f  // Top-right vertex of the square
  };

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, triangle.size() * sizeof(GLfloat),
                 triangle.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                          nullptr);
    glEnableVertexAttribArray(0);

    glUseProgram(shaderProgram1);
    const GLuint offsetLocation1 =
        glGetUniformLocation(shaderProgram1, "offset");
    glUniform2f(offsetLocation1, 0.1f, 0.1f);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glUseProgram(shaderProgram2);
    const GLuint offsetLocation2 =
        glGetUniformLocation(shaderProgram2, "offset");
    glUniform2f(offsetLocation2, -0.1f, -0.1f);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    GLuint VAO_square;
    glGenVertexArrays(1, &VAO_square);
    glBindVertexArray(VAO_square);

    GLuint VBO_square;
    glGenBuffers(1, &VBO_square);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_square);
    glBufferData(GL_ARRAY_BUFFER, square.size() * sizeof(GLfloat),
                 square.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                          nullptr);
    glEnableVertexAttribArray(0);

    glUniform2f(offsetLocation2, -0.4f, -0.4f);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glDeleteVertexArrays(1, &VAO_square);
    glDeleteBuffers(1, &VBO_square);

    glfwSwapBuffers(window);
  }

  glDeleteProgram(shaderProgram1);
  glDeleteProgram(shaderProgram2);
  glfwTerminate();

  return EXIT_SUCCESS;
}
