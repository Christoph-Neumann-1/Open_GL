/**
 * @file
 * No, I won't document this file.
 */

#pragma once

#include <Logger.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <Shader.hpp>
#include <glad/glad.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Image/stb_image.h>
#include <Buffer.hpp>
#include <VertexArray.hpp>

namespace GL
{
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 normal;
        glm::vec2 texcoord;
    };

    struct s_Texture
    {
        u_int id;
        std::string type;
        std::string path;
    };

    //TODO allow copying of data
    /**
 * @brief A bunch of vertices + textures
 */
    class Mesh
    {
        Buffer vb, ib;
        VertexArray va;

    public:
        std::vector<Vertex> vertices;
        std::vector<u_int> indices;
        std::vector<s_Texture> textures;

        Mesh(const std::vector<Vertex> &_vertices, const std::vector<u_int> &_indices, const std::vector<s_Texture> &_textures);

        /**
          * @brief Add a new buffer with instance data.
          * 
          * Used in the star scene, for example.
          * 
          */
        void AddInstanceBuffer(const VertexBufferLayout &layout, u_int Buffer);

        void AddInstanceBuffer(const VertexBufferLayout &layout, Buffer &Buffer);

        void Draw(Shader &shader, uint count = 0);

        ~Mesh()
        {
        }
    };

    class Model
    {
        Logger log;
        std::vector<Mesh> meshes;
        std::vector<s_Texture> loaded;

        std::vector<s_Texture> LoadTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

        void ProcessMesh(aiMesh *mesh, const aiScene *scene);

        std::string directory;

        void ProcessNode(aiNode *node, const aiScene *scene)
        {
            for (unsigned int i = 0; i < node->mNumMeshes; i++)
            {
                aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                ProcessMesh(mesh, scene);
            }
            for (unsigned int i = 0; i < node->mNumChildren; i++)
            {
                ProcessNode(node->mChildren[i], scene);
            }
        }

    public:
        Model(const std::string &path);

        void AddInstanceBuffer(const VertexBufferLayout &layout, u_int Buffer)
        {
            for (auto &mesh : meshes)
            {
                mesh.AddInstanceBuffer(layout, Buffer);
            }
        }

        void Draw(Shader &shader, uint count = 0)
        {
            for (auto &mesh : meshes)
                mesh.Draw(shader, count);
        }
    };
}