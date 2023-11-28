#include "assimp/Importer.hpp"
#include "assimp/material.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/types.h"

// Fixes:
// undefined reference to `stbi_load_from_memory'
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <array>
#include <functional>
#include <iostream>
#include <unordered_set>

namespace gl_model {

std::string texture_type_string(const aiTextureType texture_type) {
  switch (texture_type) {
  case aiTextureType_NONE:
    return "aiTextureType_NONE";
  case aiTextureType_DIFFUSE:
    return "aiTextureType_DIFFUSE";
  case aiTextureType_SPECULAR:
    return "aiTextureType_SPECULAR";
  case aiTextureType_AMBIENT:
    return "aiTextureType_AMBIENT";
  case aiTextureType_EMISSIVE:
    return "aiTextureType_EMISSIVE";
  case aiTextureType_HEIGHT:
    return "aiTextureType_HEIGHT";
  case aiTextureType_NORMALS:
    return "aiTextureType_NORMALS";
  case aiTextureType_SHININESS:
    return "aiTextureType_SHININESS";
  case aiTextureType_OPACITY:
    return "aiTextureType_OPACITY";
  case aiTextureType_DISPLACEMENT:
    return "aiTextureType_DISPLACEMENT";
  case aiTextureType_LIGHTMAP:
    return "aiTextureType_LIGHTMAP";
  case aiTextureType_REFLECTION:
    return "aiTextureType_REFLECTION";
  case aiTextureType_BASE_COLOR:
    return "aiTextureType_BASE_COLOR";
  case aiTextureType_NORMAL_CAMERA:
    return "aiTextureType_NORMAL_CAMERA";
  case aiTextureType_EMISSION_COLOR:
    return "aiTextureType_EMISSION_COLOR";
  case aiTextureType_METALNESS:
    return "aiTextureType_METALNESS";
  case aiTextureType_DIFFUSE_ROUGHNESS:
    return "aiTextureType_DIFFUSE_ROUGHNESS";
  case aiTextureType_AMBIENT_OCCLUSION:
    return "aiTextureType_AMBIENT_OCCLUSION";
  case aiTextureType_SHEEN:
    return "aiTextureType_SHEEN";
  case aiTextureType_CLEARCOAT:
    return "aiTextureType_CLEARCOAT";
  case aiTextureType_TRANSMISSION:
    return "aiTextureType_TRANSMISSION";
  case aiTextureType_UNKNOWN:
    return "aiTextureType_UNKNOWN";
  default:
    // Not sure if i should throw or not. Probably a design decision.
    return "Unknown Texture Type";
  }
}

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

      out << "Texture coords:\n";
      // retrieve texture coordinates for the current vertex
      for (unsigned int textureCoordIdx{0};
           textureCoordIdx < AI_MAX_NUMBER_OF_TEXTURECOORDS;
           ++textureCoordIdx) {
        if (mesh->HasTextureCoords(textureCoordIdx)) {
          out << "  Texture coord: " << textureCoordIdx << "\n";
          // retrieve uv coordinates for all vertices of the current texture
          for (unsigned int vertexIdx{0}; vertexIdx < mesh->mNumVertices;
               ++vertexIdx) {
            out << "    - Coord (vertexIdx = " << vertexIdx
                << "): " << mesh->mTextureCoords[textureCoordIdx][vertexIdx].x
                << " " << mesh->mTextureCoords[textureCoordIdx][vertexIdx].y
                << "\n";
          }
        } else {
          // Possibly check when does this case occur as we don't handle it.
          break; // Unlikely that there will be a next one.
        }
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
              out << "Textures of type ( " << texture_type_string(texture_type)
                  << " ): " << material->GetTextureCount(texture_type) << "\n";
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
                  out << "Texture ( Embedded )\n";

                  const aiTexture *texture{
                      scene->GetEmbeddedTexture(path.C_Str())};
                  out << "  - Filename: " << texture->mFilename.C_Str() << "\n";
                  out << "  - Width: " << texture->mWidth << "\n";
                  out << "  - Height: " << texture->mHeight << "\n";
                  out << "  - Format hint: " << texture->achFormatHint << "\n";

                  // For reason unknown to me texture is compressed. We would
                  // use image library anyway to handle textures, I think.
                  const bool is_compressed{texture->mHeight == 0};
                  // decompress texture
                  if (is_compressed) {
                    out << "  - Compressed\n";

                    int width{}, height{}, channels{};
                    // Assimp documentation:
                    //
                    // If mHeight = 0 this is a pointer to a memory buffer of
                    // size mWidth containing the compressed texture data. Good
                    // luck, have fun!
                    unsigned char *data{stbi_load_from_memory(
                        reinterpret_cast<const stbi_uc *>(texture->pcData),
                        texture->mWidth, &width, &height, &channels, 0)};

                    if (data) {
                      // We should be able to use texture data for rendering.

                      // The texture looks like it's in the original size.
                      // Should I have some information how it's mapped to the
                      // model? Maybe a UV map or something else?

                      out << "  Decompressed texture\n";
                      out << "    - Width: " << width << "\n";
                      out << "    - Height: " << height << "\n";
                      out << "    - Channels: " << channels << "\n";
                      stbi_image_free(data);
                    } else {
                      out << "  Failed to decompress texture: "
                          << stbi_failure_reason() << "\n";
                    }
                  } else {
                    out << "  - Decompressed\n";
                  }
                } else {
                  out << "  Texture ( Referenced )\n";
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

void inspect_model(const std::string &path, std::ostream &out = std::cout) {
  Assimp::Importer importer;

  const aiScene *scene{importer.ReadFile(path, aiProcess_Triangulate |
                                                   aiProcess_FlipUVs |
                                                   aiProcess_EmbedTextures)};

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    throw std::runtime_error("Error. Load model " + path +
                             importer.GetErrorString());
  }

  out << "Success. Load model " << path << ".\n";

  out << "Traverse " << path << "\n";
  traverse(scene, out);
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

  const std::unordered_set<std::string> paths{
      // Inspect the model's vertices
      "/home/srecko/Documents/blender/export/cube.glb",
      // Inspect enlarged mode's vertices
      "/home/srecko/Documents/blender/export/enlarged-cube.glb",
      // Inspect exported model's textures
      "/home/srecko/Documents/blender/export/cube-two.glb"};

  std::for_each(paths.begin(), paths.end(), [](const std::string &path) {
    try {
      gl_model::inspect_model(path);
    } catch (const std::exception &e) {
      std::cerr << e.what() << "\n";
    }
  });

  return 0;
}
