#include <Scene.hpp>
#include <Shader.hpp>
#include <Data.hpp>
#include <Logger.hpp>
#include <glad/glad.h>
#include <Buffer.hpp>
#include <VertexArray.hpp>

using namespace GL;

class Transparency final : public Scene
{
    Buffer VBO;
    Buffer InstanceInfo;
    VertexArray VAO;
    //Renders the layer to a texture
    Shader layerShader{"shader/Transparent.vs", "shader/Transparent.fs"};
    //Takes the layers and calculates the final image
    Shader combineShader{"shader/TransparentCombine.vs", "shader/TransparentCombine.fs"};
    uint FBOs[4];
    uint depthBuffers[4];
    uint colorBuffers[4];

    float vertices[6] = {-0.5, -0.5,
                         0.5, -0.5,
                         0, 0.5};

    const glm::vec2 square_vertices[6]{
        {-1.0f, -1.0f},
        {1.0f, -1.0f},
        {1.0f, 1.0f},
        {1.0f, 1.0f},
        {-1.0f, 1.0f},
        {-1.0f, -1.0f}};

    struct InstanceData
    {
        glm::vec4 color;
        glm::vec3 position;
    };

    void Render()
    {
        //TODO: Don't look up uniforms every frame
        VAO.Bind();
        layerShader.Bind();
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_BLEND);

        for (int i = 0; i < 4; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, FBOs[i]);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            if (i != 0)
            {
                glBindTexture(GL_TEXTURE_2D, depthBuffers[i - 1]);
                layerShader.SetUniform1i("u_firstLayer", false);
            }
            else
            {
                layerShader.SetUniform1i("u_firstLayer", true);
            }
            glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 4);
        }

        auto &color = loader->GetWindow().bgcolor;

        glClearColor(color.r, color.g, color.b, color.a);

        glEnable(GL_BLEND);

        //TODO: Do everything in one pass
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        combineShader.Bind();
        for (int i = 3; i >= 0; i--)
        {
            glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
            glDrawArrays(GL_TRIANGLES, 3, 6);
        }

        VertexArray::Unbind();
        layerShader.UnBind();
    }

public:
    Transparency(SceneLoader *_loader) : Scene(_loader)
    {
        VAO.Bind();
        VBO.Bind(GL_ARRAY_BUFFER);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(square_vertices), nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(square_vertices), square_vertices);

        VertexBufferLayout layout;
        layout.Push(GL_FLOAT, 2);
        layout.AddToVertexArray(VAO);

        InstanceData instances[4]{
            {glm::vec4(0.0f, 0.0f, 1.0f, 0.5f), glm::vec3(0.09f, 0.0f, 1.0f)},
            {glm::vec4(0.0f, 1.0f, 0.0f, 0.5f), glm::vec3(0.06f, 0.0f, 1.0f)},
            {glm::vec4(1.0f, 0.0f, 0.0f, 0.5f), glm::vec3(0.03f, 0.0f, 1.0f)},
            {glm::vec4(1.0f, 1.0f, 0.0f, 0.5f), glm::vec3(0.12f, 0.0f, 1.0f)}};

        InstanceInfo.Bind(GL_ARRAY_BUFFER);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(InstanceData), instances, GL_STATIC_DRAW);
        VertexBufferLayout layout2;
        layout2.Push(GL_FLOAT, 4);
        layout2.Push(GL_FLOAT, 3, offsetof(InstanceData, position));
        layout2.attribdivisor = 1;
        layout2.AddToVertexArray(VAO);

        RegisterFunc(CallbackType::Render, &Transparency::Render, this);

        VertexArray::Unbind();
        Buffer::Unbind(GL_ARRAY_BUFFER);

        auto width = loader->GetWindow().GetWidth();
        auto height = loader->GetWindow().GetHeight();

        glGenFramebuffers(4, FBOs);
        glGenTextures(4, colorBuffers);
        glGenTextures(4, depthBuffers);
        for (int i = 0; i < 4; i++)
        {
            glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, FBOs[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //TODO: Try GL_LINEAR
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_2D, depthBuffers[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffers[i], 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffers[i], 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        combineShader.Bind();
        combineShader.SetUniform2f("u_screenSize", width, height);
    }

    ~Transparency()
    {
        glDeleteFramebuffers(4, FBOs);
        glDeleteTextures(4, colorBuffers);
        glDeleteTextures(4, depthBuffers);
    }
};

SCENE_LOAD_FUNC(Transparency)