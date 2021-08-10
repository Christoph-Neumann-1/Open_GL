/**
 * @file ModelLoader.cpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#include <ModelLoader.hpp>

namespace GL
{
    static unsigned int LoadTextureFromFile(const char *path, const std::string &directory)
    {
        auto filename = directory + '/' + path;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;
            else
                format = 0;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            Logger log;
            log << "Texture failed to load at path: " << path;
            log.print();
            stbi_image_free(data);
        }

        return textureID;
    }

    Mesh::Mesh(const std::vector<Vertex> &_vertices, const std::vector<u_int> &_indices, const std::vector<s_Texture> &_textures)
        : vertices(_vertices), indices(_indices), textures(_textures)
    {
        vb.Bind(GL_ARRAY_BUFFER);
        ib.Bind(GL_ELEMENT_ARRAY_BUFFER);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), &indices[0], GL_STATIC_DRAW);

        va.Bind();

        VertexBufferLayout layout;
        layout.Push({GL_FLOAT,3,0});
        layout.Push({GL_FLOAT,3,(void *)offsetof(Vertex, normal)});
        layout.Push({GL_FLOAT,2,(void *)offsetof(Vertex, texcoord)});
        layout.stride = sizeof(Vertex);
        layout.AddToVertexArray(va);


        Buffer::Unbind(GL_ARRAY_BUFFER);
        Buffer::Unbind(GL_ELEMENT_ARRAY_BUFFER);
        VertexArray::Unbind();
    }

    //I keep this here in case the buffer class is insufficient
    void Mesh::AddInstanceBuffer(VertexBufferLayout &layout, u_int Buffer)
    {
        va.Bind();
        glBindBuffer(GL_ARRAY_BUFFER, Buffer);
        layout.attribdivisor=1;
        layout.AddToVertexArray(va);
        Buffer::Unbind(GL_ARRAY_BUFFER);
        VertexArray::Unbind();
    }

    ///This has the same purpose as the above function, but uses the buffer class. Both work exactly the same as 
    ///the Buffer class will be cast to an uint implicitly.
    void Mesh::AddInstanceBuffer(VertexBufferLayout &layout, Buffer &Buffer)
    {
        AddInstanceBuffer(layout, static_cast<u_int>(Buffer));
    }

    void Mesh::Draw(Shader &shader, uint count)
    {
        va.Bind();
        ib.Bind(GL_ELEMENT_ARRAY_BUFFER);

        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);

            //Figures out the name of the texture uniform
            std::string index;
            std::string textype = textures[i].type;
            if (textype == "texture_diffuse")
                index = std::to_string(diffuseNr++);
            else if (textype == "texture_specular")
                index = std::to_string(specularNr++); // transfer unsigned int to stream
            else if (textype == "texture_normal")
                index = std::to_string(normalNr++); // transfer unsigned int to stream
            else if (textype == "texture_height")
                index = std::to_string(heightNr++); // transfer unsigned int to stream

            shader.SetUniform1i(textype + index, i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        if (count)
            glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, count);
        else
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

        Buffer::Unbind(GL_ELEMENT_ARRAY_BUFFER);
        VertexArray::Unbind();
    }

    std::vector<s_Texture> Model::LoadTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        std::vector<s_Texture> textures;

        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < loaded.size(); j++)
            {
                if (std::strcmp(loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            { // if texture hasn't been loaded already, load it
                s_Texture texture;
                texture.id = LoadTextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                loaded.push_back(texture); // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }

    void Model::ProcessMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<s_Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
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

            if (mesh->mTextureCoords[0])
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
        // now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // process materials
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        std::vector<s_Texture> diffuseMaps = LoadTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        std::vector<s_Texture> specularMaps = LoadTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<s_Texture> normalMaps = LoadTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<s_Texture> heightMaps = LoadTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        meshes.emplace_back(vertices, indices, textures);
    }

    Model::Model(const std::string &path)
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
};