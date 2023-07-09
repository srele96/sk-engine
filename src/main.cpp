#include <iostream>
#include <vector>

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"

// Vertex Shader
const GLchar *vertexShaderSource{R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform vec2 offset;
    void main() {
        gl_Position = vec4(aPos.x + offset.x, aPos.y + offset.y, aPos.z, 1.0);
    }
)glsl"};

// Red
const GLchar *fragmentShaderSource1{R"glsl(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red color
    }
)glsl"};

// Blue
const GLchar *fragmentShaderSource2{
    R"glsl(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f); // Blue color
    }
)glsl"};

std::vector<GLfloat> vertices{-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                              0.0f,  0.0f,  0.5f, 0.0f};

int main() {
  // Initialize GLFW
  if (!glfwInit()) {
    std::cout << "Failed to initialize GLFW" << std::endl;
    return -1;
  }

  // Specify we want OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a windowed mode window and its OpenGL context
  GLFWwindow *window =
      glfwCreateWindow(800, 600, "GLFW Glad Test Window", NULL, NULL);
  if (!window) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);

  // Load OpenGL functions using glad
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // Vertex array object
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  // Vertex buffer object
  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
               vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // Generate shaders for box1 and box2
  unsigned int vertexShader, fragmentShader1, fragmentShader2;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  fragmentShader1 = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader1, 1, &fragmentShaderSource1, NULL);
  glCompileShader(fragmentShader1);

  fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader2, 1, &fragmentShaderSource2, NULL);
  glCompileShader(fragmentShader2);

  // Generate shader programs
  unsigned int shaderProgram1, shaderProgram2;
  shaderProgram1 = glCreateProgram();
  glAttachShader(shaderProgram1, vertexShader);
  glAttachShader(shaderProgram1, fragmentShader1);
  glLinkProgram(shaderProgram1);

  shaderProgram2 = glCreateProgram();
  glAttachShader(shaderProgram2, vertexShader);
  glAttachShader(shaderProgram2, fragmentShader2);
  glLinkProgram(shaderProgram2);

  // Delete shaders as they're linked into our program now and no longer
  // necessary
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader1);
  glDeleteShader(fragmentShader2);

  // Offset locations for both shader programs
  int offsetLocation1 = glGetUniformLocation(shaderProgram1, "offset");
  int offsetLocation2 = glGetUniformLocation(shaderProgram2, "offset");

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    // Clear the colorbuffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw the first triangle using the first shader program
    glUseProgram(shaderProgram1);
    glUniform2f(offsetLocation1, 0.1f,
                0.1f); // Use a different offset for each triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Draw the second triangle using the second shader program
    glUseProgram(shaderProgram2);
    glUniform2f(offsetLocation2, -0.1f,
                -0.1f); // Use a different offset for each triangle
    glDrawArrays(GL_TRIANGLES, 0,
                 3); // We just change the shader, not the vertices

    // Swap buffers and poll IO events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // De-allocate all resources
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shaderProgram1);
  glDeleteProgram(shaderProgram2);

  // Terminate GLFW
  glfwTerminate();
  return 0;
}
