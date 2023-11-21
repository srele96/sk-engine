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

#include <algorithm>
#include <iostream>

// I need to understand gltf model structure to be able to render it.
// 
// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#foreword
//
// https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/gltfTutorial_002_BasicGltfStructure.md

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

  return 0;
}
