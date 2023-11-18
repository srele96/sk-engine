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
      std::for_each(
          texture_types.begin(), texture_types.end(),
          [&material, &out, &scene](aiTextureType texture_type) {
            const unsigned int texture_count{
                material->GetTextureCount(texture_type)};
            if (texture_count > 0) {
              out << "Textures: " << material->GetTextureCount(texture_type)
                  << "\n";
              // All textures are 0. Why?
              // They were 0 because model didn't contain reference or
              // embedding of any texture.
              // Blender didn't export texture on diffuse bsdf node.
              // Sadge!
              for (unsigned int j{0};
                   j < material->GetTextureCount(texture_type); ++j) {
                aiString path;
                material->GetTexture(texture_type, j, &path);
                out << "Texture info:\n";
                out << "  - Path: " << path.C_Str() << "\n";
                // If texture is embedded, get it
                // assimp labels textures with '*1', '*2', etc.
                // Example: https://github.com/assimp/assimp-net/issues/50
                if (path.data[0] == '*') {
                  const aiTexture *texture{
                      scene->GetEmbeddedTexture(path.C_Str())};
                  out << "  - Filename: " << texture->mFilename.C_Str() << "\n";
                  out << "  - Width: " << texture->mWidth << "\n";
                  out << "  - Height: " << texture->mHeight << "\n";
                }
              }
            } else {
              out << "Textures: Not found\n";
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
  // --------------------------------------------------------------------------
  // Impression 1:
  //
  // All vertex data is between -1 and 1, why? I did not modify the mode's
  // scale, maybe that's the default size of the cube in blender?
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // Impression 2:
  //
  // Vertices are larger than -1 and 1. Probably because the model has to be
  // moved from model space to world space.
  //
  // Well, well... I wonder how am I going to do that! We shall see.
  // --------------------------------------------------------------------------

  { // Inspect the model's vertices
    Assimp::Importer importer;

    const std::string path{"/home/srecko/Documents/blender/export/cube.glb"};
    const aiScene *scene{importer.ReadFile(path, aiProcess_Triangulate |
                                                     aiProcess_FlipUVs |
                                                     aiProcess_EmbedTextures)};

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
      std::cerr << "Error. Load model " << path << importer.GetErrorString()
                << "\n";

      return 1;
    }

    std::cout << "Success. Load model " << path << ".\n";

    std::cout << "Traverse " << path << "\n";
    gl_model::traverse(scene);
  }

  // --------------------------------------------------------------------------

  { // Inspect enlarged mode's vertices
    Assimp::Importer importer;

    const std::string path{
        "/home/srecko/Documents/blender/export/enlarged-cube.glb"};
    const aiScene *scene{importer.ReadFile(path, aiProcess_Triangulate |
                                                     aiProcess_FlipUVs |
                                                     aiProcess_EmbedTextures)};

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
      std::cerr << "Error. Load model " << path << importer.GetErrorString()
                << "\n";

      return 1;
    }

    std::cout << "Success. Load model " << path << ".\n";

    std::cout << "Traverse " << path << "\n";
    gl_model::traverse(scene);
  }

  // --------------------------------------------------------------------------

  { // Inspect exported model's textures
    Assimp::Importer importer;

    const std::string path{
        "/home/srecko/Documents/blender/export/cube-two.glb"};
    const aiScene *scene{importer.ReadFile(path, aiProcess_Triangulate |
                                                     aiProcess_FlipUVs |
                                                     aiProcess_EmbedTextures)};

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
      std::cerr << "Error. Load model " << path << importer.GetErrorString()
                << "\n";

      return 1;
    }

    std::cout << "Success. Load model " << path << ".\n";

    std::cout << "Traverse " << path << "\n";
    gl_model::traverse(scene);
  }

  return 0;
}
