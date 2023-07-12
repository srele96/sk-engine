#ifndef __SRC_SHADER_H__
#define __SRC_SHADER_H__

// Reference:
// https://stackoverflow.com/questions/1135841/c-multiline-string-literal

#include <string>

namespace shader {
namespace vertex {

const std::string translateByOffset{R"glsl(
  #version 430 core
  layout (location = 0) in vec3 aPos;
  uniform vec2 offset;
  void main() {
    gl_Position = vec4(aPos.x + offset.x, aPos.y + offset.y, aPos.z, 1.0);
  }
)glsl"};

// I didn't know that there is concept of active and inactive uniforms within
// GLSL shaders. Active uniform is used, for example matrix multiplies vector. A
// declaration without usage is inactive uniform and we can't query it.
//
// A useless arbitrary shader with same principle for uniforms to test out
// glGetActiveUniforms.
const std::string useManyUniforms{R"glsl(
  #version 430 core

  layout (location = 0) in vec3 aPos;

  uniform int first;
  uniform float hello;
  uniform vec3 a;
  uniform mat4 woahCowboy;
  uniform mat3 nope;

  void main() {
    vec3 r1 = a * nope;
    vec4 r2 = vec4(r1.xyz, hello) + vec4(float(first), 0, 0, 0);
    // I had to assign the result of uniforms to gl_Position to make them
    // active.
    gl_Position = woahCowboy * vec4(aPos.xyz, 1.0) * r2;
  }
)glsl"};

} // namespace vertex

namespace fragment {

const std::string red{R"glsl(
  #version 430 core
  out vec4 FragColor;
  void main() {
    FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
  }
)glsl"};

const std::string blue{R"glsl(
  #version 430 core
  out vec4 FragColor;
  void main() {
    FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
  }
)glsl"};

} // namespace fragment
} // namespace shader

#endif // __SRC_SHADER_H__