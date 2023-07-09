#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"

// Vertex data for a unit sphere
const float radius = 0.5f;
const int sectorCount = 36;
const int stackCount = 18;

struct Vertex {
  float position[3];
};

int main() {
  // Initialize GLFW
  if (!glfwInit()) {
    return 1;
  }

  // Specify we want OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a windowed mode window and its OpenGL context
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "GLFW Glad Test Window", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return 1;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);

  // Load OpenGL functions using glad
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    return 1;
  }

  // Define vertex data for a unit sphere
  std::vector<Vertex> vertices;
  std::vector<GLuint> indices;

  for (int i = 0; i <= stackCount; ++i) {
    float stackAngle = M_PI / 2 - i * (M_PI / stackCount);
    float xy = radius * std::cos(stackAngle);
    float z = radius * std::sin(stackAngle);

    for (int j = 0; j <= sectorCount; ++j) {
      float sectorAngle = j * (2 * M_PI / sectorCount);
      float x = xy * std::cos(sectorAngle);
      float y = xy * std::sin(sectorAngle);

      Vertex vertex;
      vertex.position[0] = x;
      vertex.position[1] = y;
      vertex.position[2] = z;

      vertices.push_back(vertex);
    }
  }

  for (int i = 0; i < stackCount; ++i) {
    int k1 = i * (sectorCount + 1);
    int k2 = k1 + sectorCount + 1;

    for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
      indices.push_back(k1);
      indices.push_back(k2);
      indices.push_back(k1 + 1);
      indices.push_back(k2);
      indices.push_back(k2 + 1);
      indices.push_back(k1 + 1);
    }
  }

  // Create a vertex buffer object (VBO) to store the vertex data
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
               vertices.data(), GL_STATIC_DRAW);

  // Create an element buffer object (EBO) to store the index data
  GLuint ebo;
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
               indices.data(), GL_STATIC_DRAW);

  // Create a vertex array object (VAO) to encapsulate the VBO and EBO bindings
  // and attribute configurations
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

  // Create and compile shaders for the vertex and fragment stages
  const char *vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPosition;
        void main() {
            gl_Position = vec4(aPosition, 1.0);
        }
    )";
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  const char *fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.5, 0.2, 1.0);
        }
    )";
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  // Create a shader program and link the vertex and fragment shaders
  GLuint shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  glUseProgram(shaderProgram);

  // Loop until the user closes the window
  while (!glfwWindowShouldClose(window)) {
    // Set clear color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    // Clear the buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the sphere
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  // Cleanup
  glDeleteProgram(shaderProgram);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
  glDeleteVertexArrays(1, &vao);

  glfwTerminate();
  return 0;
}
