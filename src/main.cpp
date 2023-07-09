#include <iostream>
#include <vector>

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

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

std::vector<GLfloat> triangle{-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                              0.0f,  0.0f,  0.5f, 0.0f};
std::vector<GLfloat> square{
    -0.5f, -0.5f, 0.0f, // Bottom-left vertex of the square
    0.5f,  -0.5f, 0.0f, // Bottom-right vertex of the square
    -0.5f, 0.5f,  0.0f, // Top-left vertex of the square
    0.5f,  0.5f,  0.0f  // Top-right vertex of the square
};

// Frame timing
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

const int initialWidth = 800;
const int initialHeight = 600;
int width = initialWidth;
int height = initialHeight;

void onSetCursorPosEvent(GLFWwindow *window, double xpos, double ypos) {
  // TODO:
}

void onSetKeyEvent(GLFWwindow *window, int key, int scancode, int action,
                   int mods) {
  // TODO:
}

void onSetWindowSize(GLFWwindow *window, int w, int h) {
  width = w;
  height = h;
}

void updateDeltaTime() {
  GLfloat currentFrame = glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;
}

int main() {
  // Initialize GLFW
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return EXIT_FAILURE;
  }

  // Specify we want OpenGL 3.3
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create a windowed mode window and its OpenGL context
  GLFWwindow *window =
      glfwCreateWindow(width, height, "GLFW Glad Test Window", NULL, NULL);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return EXIT_FAILURE;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);

  // Load OpenGL functions using glad
  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cerr << "Failed to initialize GLAD\n";
    return EXIT_FAILURE;
  }

  // Generate shaders for box1 and box2
  GLuint vertexShader, fragmentShader1, fragmentShader2;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);

  fragmentShader1 = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader1, 1, &fragmentShaderSource1, nullptr);
  glCompileShader(fragmentShader1);

  fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader2, 1, &fragmentShaderSource2, nullptr);
  glCompileShader(fragmentShader2);

  // Generate shader programs
  GLuint shaderProgram1, shaderProgram2;
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
  GLuint offsetLocation1 = glGetUniformLocation(shaderProgram1, "offset");
  GLuint offsetLocation2 = glGetUniformLocation(shaderProgram2, "offset");

  glfwSetCursorPosCallback(window, onSetCursorPosEvent);
  glfwSetKeyCallback(window, onSetKeyEvent);
  glfwSetWindowSizeCallback(window, onSetWindowSize);

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    updateDeltaTime();

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    // Clear the color buffer and depth buffer
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Vertex array object
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Vertex buffer object
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, triangle.size() * sizeof(GLfloat),
                 triangle.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                          nullptr);
    glEnableVertexAttribArray(0);

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

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // Vertex buffer object for square
    GLuint VAO_square;
    glGenVertexArrays(1, &VAO_square);
    glBindVertexArray(VAO_square);

    // Vertex buffer object
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

    // Swap buffers and poll IO events
    glfwSwapBuffers(window);
  }

  glDeleteProgram(shaderProgram1);
  glDeleteProgram(shaderProgram2);

  // Terminate GLFW
  glfwTerminate();
  return EXIT_SUCCESS;
}
