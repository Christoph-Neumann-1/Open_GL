#include <Scene.hpp>
#include <Buffer.hpp>
#include <VertexArray.hpp>
#include <Shader.hpp>

using namespace GL;

class FrameBufferDemo : public Scene
{
    Buffer vertexBuffer, vertexBuffer2;
    VertexArray vao, vao2;
    Buffer indexBuffer, indexBuffer2;
    uint fbo;
    uint texture;
    uint rbo;

    Shader firstShader{"shader/Default.vs", "shader/Default.fs"};
    Shader secondShader{"shader/FBO.vs", "shader/FBO.fs"};

    void Render()
    {
        auto oldbg = loader->GetWindow().bgcolor;
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        vao.Bind();
        indexBuffer.Bind(GL_ELEMENT_ARRAY_BUFFER);
        firstShader.Bind();
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        loader->GetWindow().bgcolor = oldbg;
        vao2.Bind();
        indexBuffer2.Bind(GL_ELEMENT_ARRAY_BUFFER);
        secondShader.Bind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        VertexArray::Unbind();
        Buffer::Unbind(GL_ELEMENT_ARRAY_BUFFER);
        secondShader.UnBind();
        glBindTexture(GL_TEXTURE_2D, 0);
    }

public:
    FrameBufferDemo(SceneLoader *_loader) : Scene(_loader)
    {
        RegisterFunc(CallbackType::Render, &FrameBufferDemo::Render, this);

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loader->GetWindow().GetWidth(), loader->GetWindow().GetHeigth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, loader->GetWindow().GetWidth(), loader->GetWindow().GetHeigth());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            Logger()("Error framebuffer incomplete");
            loader->UnLoad();
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glm::vec2 vertices[] = {
            glm::vec2(-0.5f, -0.5f),
            glm::vec2(0.5f, -0.5f),
            glm::vec2(0.5f, 0.5f),
        };
        uint indices[] = {
            0, 1, 2};

        vertexBuffer.Bind(GL_ARRAY_BUFFER);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        indexBuffer.Bind(GL_ELEMENT_ARRAY_BUFFER);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        vao.Bind();
        VertexBufferLayout layout;
        layout.Push(GL_FLOAT, 2);
        layout.AddToVertexArray(vao);
        Buffer::Unbind(GL_ARRAY_BUFFER);
        Buffer::Unbind(GL_ELEMENT_ARRAY_BUFFER);
        VertexArray::Unbind();

        firstShader.Bind();
        firstShader.SetUniform4f("u_Color", 0.0f, 0.5f, 1.0f, 1.0f);
        firstShader.SetUniformMat4f("u_MVP", glm::mat4(1.0f));
        firstShader.UnBind();

        secondShader.Bind();
        secondShader.SetUniform1i("u_Texture", 0);
        secondShader.UnBind();

        vertexBuffer2.Bind(GL_ARRAY_BUFFER);
        glm::vec4 vertices2[] = {
            glm::vec4(-1.0f, -1.0f, 0.0f,0.0f),
            glm::vec4(1.0f, -1.0f,1.0f,0.0f),
            glm::vec4(1.0f, 1.0f,1.0f,1.0f),
            glm::vec4(-1.0f, 1.0f,0.0f,1.0f),
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
        VertexBufferLayout layout2;
        layout2.Push(GL_FLOAT, 2);
        layout2.Push(GL_FLOAT, 2,2*sizeof(float));
        layout2.stride=4*sizeof(float);
        vao2.Bind();
        layout2.AddToVertexArray(vao2);
        Buffer::Unbind(GL_ARRAY_BUFFER);

        indexBuffer2.Bind(GL_ELEMENT_ARRAY_BUFFER);
        uint indices2[] = {
            0, 1, 2,
            0, 2, 3
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);
        Buffer::Unbind(GL_ELEMENT_ARRAY_BUFFER);
    }
    ~FrameBufferDemo()
    {
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &texture);
        glDeleteRenderbuffers(1, &rbo);
    }
};

SCENE_LOAD_FUNC(FrameBufferDemo)