#pragma once

#include <Log.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <Shader.hpp>
#include <glad/glad.h>
#include <VertexArray.hpp>
#include <IndexBuffer.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
};

class Mesh
{
    VertexArray va;
    VertexBuffer vb;
    IndexBuffer ib;

public:
    std::vector<Vertex> vertices;
    std::vector<u_int> indices;
    std::vector<s_Texture> textures;

    Mesh(const std::vector<Vertex> &vertices, const std::vector<u_int> &indices, const std::vector<s_Texture> &textures)
        : vb(vertices.size() * sizeof(Vertex), &vertices[0]), ib(&indices[0], indices.size()), vertices(vertices), indices(indices), textures(textures)
    {
        va.Bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), 0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texcoord));
    }

    void Draw(Shader &shader)
    {
        va.Bind();
        ib.Bind();
        shader.Bind();

        glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, 0);
    }
};

class Model
{
    Logger log;
    std::vector<Mesh> meshes;

    void ProcessMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<s_Texture> textures;

        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal = vector;
            }

            if(mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.texcoord = vec;
            }
            else
                vertex.texcoord = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);        
        }
        /*
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        
        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);*/

        meshes.emplace_back(vertices,indices,textures);
    
    }

    std::vector<s_Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
    }
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
    Model(const std::string &path)
    {
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            log << "ERROR::ASSIMP::" << import.GetErrorString();
            log.print();
            return;
        }
        directory = path.substr(0, path.find_last_of('/'));
        ProcessNode(scene->mRootNode, scene);
    }

    void Draw(Shader &shader)
    {
        for (auto &mesh : meshes)
            mesh.Draw(shader);
    }
};
