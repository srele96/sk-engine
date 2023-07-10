#include <iostream>
#include <vector>

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"

const GLchar *vertexShaderSource{R"glsl(
    #version 430 core
    layout (location = 0) in vec3 aPos;
    uniform vec2 offset;
    void main() {
        gl_Position = vec4(aPos.x + offset.x, aPos.y + offset.y, aPos.z, 1.0);
    }
)glsl"};

const GLchar *fragmentShaderSource1{R"glsl(
    #version 430 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red color
    }
)glsl"};

const GLchar *fragmentShaderSource2{
    R"glsl(
    #version 430 core
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
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(width, height, "GLFW Glad Test Window", NULL, NULL);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cerr << "Failed to initialize GLAD\n";
    return EXIT_FAILURE;
  }

  const GLubyte *glVersion{glGetString(GL_VERSION)};
  std::cout << "OpenGL Version: " << glVersion << "\n";

  GLuint vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);

  GLuint fragmentShader1 = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader1, 1, &fragmentShaderSource1, nullptr);
  glCompileShader(fragmentShader1);

  GLuint fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader2, 1, &fragmentShaderSource2, nullptr);
  glCompileShader(fragmentShader2);

  GLuint shaderProgram1 = glCreateProgram();
  glAttachShader(shaderProgram1, vertexShader);
  glAttachShader(shaderProgram1, fragmentShader1);
  glLinkProgram(shaderProgram1);

  GLuint shaderProgram2 = glCreateProgram();
  glAttachShader(shaderProgram2, vertexShader);
  glAttachShader(shaderProgram2, fragmentShader2);
  glLinkProgram(shaderProgram2);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader1);
  glDeleteShader(fragmentShader2);

  GLuint offsetLocation1 = glGetUniformLocation(shaderProgram1, "offset");
  GLuint offsetLocation2 = glGetUniformLocation(shaderProgram2, "offset");

  glfwSetCursorPosCallback(window, onSetCursorPosEvent);
  glfwSetKeyCallback(window, onSetKeyEvent);
  glfwSetWindowSizeCallback(window, onSetWindowSize);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    updateDeltaTime();

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

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
    glUniform2f(offsetLocation1, 0.1f, 0.1f);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glUseProgram(shaderProgram2);
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
