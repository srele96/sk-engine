#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/types.h"
#include <array>
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

      // Draw normals after drawing the model as an exercise.
      // print all normals
      out << "Normals: " << mesh->mNumVertices << "\n";
      for (unsigned int j{0}; j < mesh->mNumVertices; ++j) {
        out << "Normal: " << mesh->mNormals[j].x << " " << mesh->mNormals[j].y
            << " " << mesh->mNormals[j].z << "\n";
      }

      out << "Materials: " << scene->mNumMaterials << "\n";

      // https://stackoverflow.com/questions/8498300/allow-for-range-based-for-with-enum-classes
      std::array<aiTextureType, 22> texture_types{
          aiTextureType::aiTextureType_NONE,
          aiTextureType::aiTextureType_DIFFUSE,
          aiTextureType::aiTextureType_SPECULAR,
          aiTextureType::aiTextureType_AMBIENT,
          aiTextureType::aiTextureType_EMISSIVE,
          aiTextureType::aiTextureType_HEIGHT,
          aiTextureType::aiTextureType_NORMALS,
          aiTextureType::aiTextureType_SHININESS,
          aiTextureType::aiTextureType_OPACITY,
          aiTextureType::aiTextureType_DISPLACEMENT,
          aiTextureType::aiTextureType_LIGHTMAP,
          aiTextureType::aiTextureType_REFLECTION,
          aiTextureType::aiTextureType_BASE_COLOR,
          aiTextureType::aiTextureType_NORMAL_CAMERA,
          aiTextureType::aiTextureType_EMISSION_COLOR,
          aiTextureType::aiTextureType_METALNESS,
          aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS,
          aiTextureType::aiTextureType_AMBIENT_OCCLUSION,
          aiTextureType::aiTextureType_SHEEN,
          aiTextureType::aiTextureType_CLEARCOAT,
          aiTextureType::aiTextureType_TRANSMISSION,
          aiTextureType::aiTextureType_UNKNOWN};

      aiMaterial *material{scene->mMaterials[mesh->mMaterialIndex]};
      out << "Material name: " << material->GetName().C_Str() << "\n";
      // get materials
      std::for_each(texture_types.begin(), texture_types.end(),
                    [&material, &out](aiTextureType texture_type) {
                      out << "Textures: "
                          << material->GetTextureCount(texture_type) << "\n";
                      // All textures are 0. Why?
                      for (unsigned int j{0};
                           j < material->GetTextureCount(texture_type); ++j) {
                        aiString texturePath;
                        material->GetTexture(texture_type, j, &texturePath);
                        out << "Texture path: " << texturePath.C_Str() << "\n";
                      }
                    });

      aiColor3D diffuseColor;
      if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
        out << "Diffuse color: " << diffuseColor.r << " " << diffuseColor.g
            << " " << diffuseColor.b << "\n";
      } else {
        out << "Diffuse color: not found\n";
      }

      aiColor3D specularColor;
      if (material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) ==
          AI_SUCCESS) {
        out << "Specular color: " << specularColor.r << " " << specularColor.g
            << " " << specularColor.b << "\n";
      } else {
        out << "Specular color: not found\n";
      }

      // print all indices
      out << "Faces: " << mesh->mNumFaces << "\n";
      for (unsigned int j{0}; j < mesh->mNumFaces; ++j) {
        aiFace face{mesh->mFaces[j]};

        out << "Face indices: " << face.mNumIndices << "\n";
        for (unsigned int k{0}; k < face.mNumIndices; ++k) {
          out << face.mIndices[k] << " ";
        }
        out << "\n";
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
