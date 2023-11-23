// Without these defines, there is linking error:
//
// undefined reference to
// `tinygltf::TinyGLTF::LoadBinaryFromFile(tinygltf::Model*,
// std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char>
// >*, std::__cxx11::basic_string<char, std::char_traits<char>,
// std::allocator<char> >*, std::__cxx11::basic_string<char,
// std::char_traits<char>, std::allocator<char> > const&, unsigned int)'
// /usr/bin/ld: CMakeFiles/tinygltf-model.dir/main.cpp.o: in function
// `tinygltf::TinyGLTF::TinyGLTF()'
// ----------------------------------------------------------------------------
//
// https://github.com/syoyo/tinygltf#build-and-example
// Define these only in *one* .cc file.
//
// Note: Includign stb_image might cause problems in combination with macros
// below.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <algorithm>
#include <iostream>
#include <vector>

// I need to understand gltf model structure to be able to render it.
//
// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#foreword
//
// https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/gltfTutorial_002_BasicGltfStructure.md

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                         int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

class delta {
private:
  double m_last_frame_time{glfwGetTime()};
  double m_delta_time{delta::initial_delta_time};

public:
  const static double initial_delta_time{0.0};

  double delta_time() const { return m_delta_time; }

  void update() {
    const double current_frame_time{glfwGetTime()};
    const double delta_time{current_frame_time - m_last_frame_time};
    m_delta_time = delta_time;
    m_last_frame_time = current_frame_time;
  }
};

int main() {
  // ----------------------------------------------------------------------------
  // Two points:
  //
  // - Triangulation: tinygltf does not automatically flip texture coordinates
  // because it aims to stay true to the GLTF specification. If you need to flip
  // the UVs to accommodate your rendering pipeline (like OpenGL), you'll have
  // to manually adjust the texture coordinates after loading the model.
  //
  // - Triangulation: GLTF models might contain meshes with polygons that are
  // not triangles (e.g., quads). If your rendering pipeline only supports
  // triangles, you'll need these to be triangulated. tinygltf does not
  // automatically triangulate polygons into triangles. You might need to do
  // this triangulation yourself after loading the model, or use a tool that can
  // preprocess the GLTF file to convert all polygons to triangles.
  //
  // ----------------------------------------------------------------------------
  //
  // Note.
  //
  // It is not easier to use something else. There are just different problems
  // to solve.
  // ----------------------------------------------------------------------------

  tinygltf::Model model;
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  bool ret = loader.LoadBinaryFromFile(
      &model, &err, &warn,
      "/home/srecko/Documents/blender/export/cube-two.glb");

  if (!warn.empty()) {
    std::cout << "Warn: " << warn << "\n";
  }

  if (!err.empty()) {
    std::cout << "Err: " << err << "\n";
  }

  if (!ret) {
    std::cout << "Failed to parse glTF\n";
    return -1;
  }

  std::cout << "Loaded glTF\n";

  std::cout << "Asset version: " << model.asset.version << "\n";

  std::function<void(const tinygltf::Node &, const tinygltf::Model &)>
      process_node{[&](const tinygltf::Node &node,
                       const tinygltf::Model &model) {
        std::cout << "Node name: " << node.name << "\n";

        if (node.mesh >= 0) {
          const tinygltf::Mesh &mesh = model.meshes[node.mesh];

          std::cout << "Node Mesh ( name = " << mesh.name
                    << ", idx = " << node.mesh << " ):\n";

          for (const tinygltf::Primitive &primitive : mesh.primitives) {
            std::cout << "Primitive\n";

            for (const auto &[name, idx] : primitive.attributes) {
              std::cout << "Attribute: " << name << "\n";

              const tinygltf::Accessor &vertexAccessor{model.accessors[idx]};
              const tinygltf::BufferView &bufferView{
                  model.bufferViews[vertexAccessor.bufferView]};
              const tinygltf::Buffer &buffer{model.buffers[bufferView.buffer]};
              const float *vertexData{reinterpret_cast<const float *>(
                  &buffer.data[bufferView.byteOffset +
                               vertexAccessor.byteOffset])};

              std::cout << "Vertex count: " << vertexAccessor.count << "\n";

              for (int i = 0; i < vertexAccessor.count; ++i) {
                std::cout << "Vertex " << i << ": " << vertexData[i * 3 + 0]
                          << ", " << vertexData[i * 3 + 1] << ", "
                          << vertexData[i * 3 + 2] << "\n";
              }
            }

            if (primitive.indices >= 0) {
              const tinygltf::Accessor &accessor =
                  model.accessors[primitive.indices];

              std::cout << "Indices count: " << accessor.count << "\n";
            }
          }
        }

        std::cout << "Node ( children = " << node.children.size() << " )\n";

        for (const int idx : node.children) {
          const tinygltf::Node &child = model.nodes[idx];

          process_node(child, model);
        }
      }};

  for (const tinygltf::Scene &scene : model.scenes) {
    for (const int idx : scene.nodes) {
      const tinygltf::Node &node = model.nodes[idx];

      process_node(node, model);
    }
  }

  if (glfwInit() == GLFW_FALSE) {
    std::cout << "Failed to initialize GLFW\n";
    return -1;
  }

  GLFWwindow *window{
      glfwCreateWindow(800, 600, "GLFW Window", nullptr, nullptr)};

  if (window == nullptr) {
    std::cout << "Failed to create GLFW window\n";
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    std::cout << "Failed to initialize GLAD\n";
    glfwTerminate();
    return -1;
  }

  glViewport(0, 0, 800, 600);

  // All of a sudden thinking about vertices, data, shaders, buffers, is not so
  // foreign to me.

  // - For a vertex store position, color
  // - I would like to store the normal, find out how to get normal, do i have
  //   to calculate it, can i calculate it from vertex data, i just want to have
  //   normal along with position to draw normals as lines
  //

  // - create vertex array object
  // - create vertex buffer object
  // - create element buffer object

  // I wonder if i should see shaders as reusable modules, functions, etc... Can
  // i make shaders sound like some familiar concept that I like?
  // - Advice from competitive programmer, make problem sound like something i
  //   like. I like something familiar, easy to deal with, already know how to
  //   solve it.

  GLuint vao;
  glGenVertexArrays(1, &vao);

  // Is it possible to create N vertex arrays? Find some use case for it.

  GLuint vbo;
  glGenBuffers(1, &vbo);

  GLuint ebo;
  glGenBuffers(1, &ebo);

  // clang-format off
  std::vector<GLfloat> data{
      // left bottom back
      -0.5f, -0.5f, 0.5f,
      // orange
      1.0f, 0.0f, 0.0f,
      // right bottom back
      0.5f, -0.5f, 0.5f,
      // green
      0.0f, 1.0f, 0.0f,
      // center bottom front
      0.0f, 0.0f, -0.5f,
      // blue
      0.0f, 0.0f, 1.0f,
      // center up center
      0.0f, 0.5f, 0.0f,
      // yellow
      1.0f, 1.0f, 0.0f
  };
  // clang-format on

  // clang-format off
  std::vector<GLuint> indices{
      // back face
      0, 1, 3,
      // left face
      0, 2, 3,
      // right face
      0, 1, 2,
      // bottom face
      1, 2, 3
  };
  // clang-format on

  // To be able to see this pyramid i would need to move camera back, because
  // the camera is initially on (0, 0, 0) position.
  //
  // I should implement camera movement with keyboard and mouse to observe the
  // pyramid from different angles.

  // Vertex array has to be bound before binding buffers. I was receiving
  // segmentation fault because I called it after binding and passing data to
  // vertex buffer object and element buffer object.
  // It seems that I have to bind vertex array before passing data to element
  // buffer object. Why? If I bind vertex array before passing data to ebo, it
  // works. If I bind after passing data to ebo I get segfault. Why?
  glBindVertexArray(vao);

  // pass data to vbo
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(GLfloat), data.data(),
               GL_STATIC_DRAW);
  // Find a scenario where I need some other draw than GL_STATIC_DRAW. Then
  // practice that draw type.

  // pass indices to ebo
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
               indices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                        reinterpret_cast<void *>(0));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat),
                        reinterpret_cast<void *>(3 * sizeof(GLfloat)));
  glEnableVertexAttribArray(1);

  const GLchar *vs_src{R"(
#version 330 core

uniform mat4 rotation;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 color;

out vec3 vertex_color;

void main() {
  gl_Position = rotation * vec4(pos.x, pos.y, pos.z, 1.0);
  vertex_color = color;
}
)"};

  const GLchar *fs_src{R"(
#version 330 core

in vec3 vertex_color;

out vec4 frag_color;

void main() {
  frag_color = vec4(vertex_color.xyz, 1.0);
}
)"};

  GLuint vs{glCreateShader(GL_VERTEX_SHADER)};
  glShaderSource(vs, 1, &vs_src, nullptr);
  glCompileShader(vs);

  {
    GLint success;
    GLchar info_log[512];
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (success) {
      std::cout << "Vertex shader compiled successfully\n";
    } else {
      glGetShaderInfoLog(vs, 512, nullptr, info_log);
      std::cerr << "Failed to compile vertex shader\n" << info_log << "\n";
    }
  }

  GLuint fs{glCreateShader(GL_FRAGMENT_SHADER)};
  glShaderSource(fs, 1, &fs_src, nullptr);
  glCompileShader(fs);

  {
    GLint success;
    GLchar info_log[512];
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (success) {
      std::cout << "Fragment shader compiled successfully\n";
    } else {
      glGetShaderInfoLog(fs, 512, nullptr, info_log);
      std::cerr << "Failed to compile fragment shader\n" << info_log << "\n";
    }
  }

  GLuint program{glCreateProgram()};
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  {
    GLint success;
    GLchar info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success) {
      std::cout << "Program linked successfully\n";
    } else {
      glGetProgramInfoLog(program, 512, nullptr, info_log);
      std::cerr << "Failed to link program\n" << info_log << "\n";
    }
  }

  // Do I need this line? Yes, I do.
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glValidateProgram.xhtml
  glValidateProgram(program);

  {
    GLint success;
    GLchar info_log[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success) {
      std::cout << "Program validated successfully\n";
    } else {
      glGetProgramInfoLog(program, 512, nullptr, info_log);
      std::cerr << "Failed to validate program\n" << info_log << "\n";
    }
  }

  glUseProgram(program);

  delta delta_;

  const float deg_0{0.0f};
  const float deg_15{15.0f};

  float a_deg_x{0.0f};
  float a_deg_z{0.0f};

  float b_deg_x{0.0f};
  float b_deg_z{0.0f};

  while (!glfwWindowShouldClose(window)) {
    delta_.update();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
      a_deg_x += deg_0 * static_cast<float>(delta_.delta_time());
      a_deg_z += deg_15 * static_cast<float>(delta_.delta_time());

      glm::quat rotate_x{
          glm::angleAxis(glm::radians(a_deg_x), glm::vec3(1.0f, 0.0f, 0.0f))};
      glm::quat rotate_z{
          glm::angleAxis(glm::radians(a_deg_z), glm::vec3(0.0f, 0.0f, 1.0f))};
      glm::quat quat{rotate_x * rotate_z};

      glm::mat4 rotation_matrix{quat};

      GLint rotationLocation = glGetUniformLocation(program, "rotation");
      glUniformMatrix4fv(rotationLocation, 1, GL_FALSE,
                         glm::value_ptr(rotation_matrix));

      glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }

    // Can I draw the same thing again with different rotation?

    {
      b_deg_x += deg_15 * static_cast<float>(delta_.delta_time());
      b_deg_z += deg_15 * static_cast<float>(delta_.delta_time());

      glm::quat rotate_x{
          glm::angleAxis(glm::radians(b_deg_x), glm::vec3(1.0f, 0.0f, 0.0f))};
      glm::quat rotate_z{
          glm::angleAxis(glm::radians(b_deg_z), glm::vec3(0.0f, 0.0f, 1.0f))};
      glm::quat quat{rotate_x * rotate_z};

      glm::mat4 rotation_matrix{quat};

      GLint rotationLocation = glGetUniformLocation(program, "rotation");
      glUniformMatrix4fv(rotationLocation, 1, GL_FALSE,
                         glm::value_ptr(rotation_matrix));

      glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // unbind buffers

  return 0;
}
