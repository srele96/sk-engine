#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <functional>
#include <iostream>

namespace gl_model {

void traverse(const aiScene *scene, std::ostream &out = std::cout) {
  std::function<void(aiNode *)> recurse{[&scene, &recurse, &out](aiNode *node) {
    // is there a way to use c++ standard containers instead of c-style
    // arrays? process all meshes
    out << "Meshes: " << node->mNumMeshes << "\n";
    for (unsigned int i{0}; i < node->mNumMeshes; ++i) {
      aiMesh *mesh{scene->mMeshes[node->mMeshes[i]]};

      // print all vertices
      out << "Vertices: " << mesh->mNumVertices << "\n";
      for (unsigned int j{0}; j < mesh->mNumVertices; ++j) {
        out << "Vertex: " << mesh->mVertices[j].x << " " << mesh->mVertices[j].y
            << " " << mesh->mVertices[j].z << "\n";
      }

      // print all normals
      out << "Normals: " << mesh->mNumVertices << "\n";
      for (unsigned int j{0}; j < mesh->mNumVertices; ++j) {
        out << "Normal: " << mesh->mNormals[j].x << " " << mesh->mNormals[j].y
            << " " << mesh->mNormals[j].z << "\n";
      }

      // print all textures
      out << "Textures: " << mesh->mNumVertices << "\n";
      for (unsigned int j{0}; j < mesh->mNumVertices; ++j) {
        out << "Texture: " << mesh->mTextureCoords[0][j].x << " "
            << mesh->mTextureCoords[0][j].y << "\n";
      }

      // print all indices
      out << "Indices: " << mesh->mNumFaces << "\n";
      for (unsigned int j{0}; j < mesh->mNumFaces; ++j) {
        aiFace face{mesh->mFaces[j]};
        for (unsigned int k{0}; k < face.mNumIndices; ++k) {
          out << "Index: " << face.mIndices[k] << "\n";
        }
      }
    }

    for (unsigned int i{0}; i < node->mNumChildren; ++i) {
      recurse(node->mChildren[i]);
    }
  }};

  recurse(scene->mRootNode);
}

} // namespace gl_model

int main() {
  Assimp::Importer cubeImporter;

  const std::string cubePath{"/home/srecko/Documents/blender/export/cube.glb"};
  const aiScene *cubeScene{cubeImporter.ReadFile(
      cubePath, aiProcess_Triangulate | aiProcess_FlipUVs)};

  if (!cubeScene || cubeScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !cubeScene->mRootNode) {
    std::cerr << "Error. Load model " << cubePath
              << cubeImporter.GetErrorString() << "\n";

    return 1;
  }

  std::cout << "Success. Load model " << cubePath << ".\n";

  // --------------------------------------------------------------------------
  // All vertex data is between -1 and 1, why? I did not modify the mode's
  // scale, maybe that's the default size of the cube in blender?
  // --------------------------------------------------------------------------

  std::cout << "Traverse " << cubePath << "\n";
  gl_model::traverse(cubeScene);

  // --------------------------------------------------------------------------

  Assimp::Importer enlargedCubeImporter;

  const std::string enlargedCubePath{
      "/home/srecko/Documents/blender/export/enlarged-cube.glb"};
  const aiScene *enlargedCubeScene{enlargedCubeImporter.ReadFile(
      enlargedCubePath, aiProcess_Triangulate | aiProcess_FlipUVs)};

  if (!enlargedCubeScene ||
      enlargedCubeScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !enlargedCubeScene->mRootNode) {
    std::cerr << "Error. Load model " << enlargedCubePath
              << enlargedCubeImporter.GetErrorString() << "\n";

    return 1;
  }

  std::cout << "Success. Load model " << enlargedCubePath << ".\n";

  std::cout << "Traverse " << enlargedCubePath << "\n";
  gl_model::traverse(enlargedCubeScene);

  // --------------------------------------------------------------------------
  // Vertices are larger than -1 and 1. Probably because the model has to be
  // moved from model space to world space.
  //
  // Well, well... I wonder how am I going to do that! We shall see.
  // --------------------------------------------------------------------------

  return 0;
}
