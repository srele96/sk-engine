#ifndef __SRC_SHADER_H__
#define __SRC_SHADER_H__

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

} // namespace vertex

namespace fragment {

const std::string red{
    R"glsl(
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