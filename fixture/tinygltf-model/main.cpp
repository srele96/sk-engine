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
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <algorithm>
#include <iostream>

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
      process_node{
          [&](const tinygltf::Node &node, const tinygltf::Model &model) {
            std::cout << "Node name: " << node.name << "\n";

            if (node.mesh >= 0) {
              const tinygltf::Mesh &mesh = model.meshes[node.mesh];

              std::cout << "Node Mesh ( name = " << mesh.name
                        << ", idx = " << node.mesh << " ):\n";

              for (const tinygltf::Primitive &primitive : mesh.primitives) {
                std::cout << "Primitive\n";

                for (const auto &attrib : primitive.attributes) {
                  std::cout << "Attribute: " << attrib.first << "\n";
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
