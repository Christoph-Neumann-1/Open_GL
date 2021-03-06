/**
 * @file Breakout.cpp
 * @brief The ball bounces off the rectangle thing and destroys the bricks
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#include <Scene.hpp>
#include <Shader.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Data.hpp>
#include <Input.hpp>
#include <Logger.hpp>
#include <random>
#include <glm/gtx/rotate_vector.hpp>
#include <Image/stb_image.h>
#include <Buffer.hpp>
#include <VertexArray.hpp>
#include <Particle.hpp>

//TODO: slight variation in color of particles

using namespace GL;

class Breakout : public Scene
{
#pragma region Variables
    Shader shader{"shader/Default.vs", "shader/Default.fs"};
    Shader bshader{"shader/Boxes.vs", "shader/Boxes.fs"};

    VertexArray va;
    Buffer vb;
    Buffer instance_buffer;

    glm::mat4 ortho;

    glm::vec2 b_pos;
    glm::vec2 b_vel;
    glm::vec4 b_color;
    const float bradius = 0.045f;

    glm::vec2 vertices[32 + 2];

    glm::vec2 bounds;

    int points = 0;

    float barx = 0.0f;

    float bary = -0.9f;

    const glm::vec2 bar_size{0.4f, 0.04f};

    const glm::vec2 square_vertices[6]{
        {-1.0f, -1.0f},
        {1.0f, -1.0f},
        {1.0f, 1.0f},
        {1.0f, 1.0f},
        {-1.0f, 1.0f},
        {-1.0f, -1.0f}};
    const float speedincrease = 0.02f;

    const float centerLineWidth = 0.005f;

    struct BufferElement
    {
        glm::vec2 pos;
        glm::vec4 color;
        BufferElement(glm::vec2 p, glm::vec4 c) : pos(p), color(c) {}
    };

    std::vector<BufferElement> boxes;

    const uint rows = 9;
    const uint cols = 10;

    const glm::vec2 box_size{2.0f / cols, 1.0f / rows};

    uint BG;
    Shader bgshader{"shader/BG.vs", "shader/BG.fs"};

    ParticleContainer2D particles{200};
    const float particle_size = 20.0f;
    const float particle_size_variance = 3.0f;
    const float particle_speed_avg = 0.1f;
    const float particle_speed_dev = 0.08f;
    const float particle_lifetime = 5.0f;
    const float particle_lifetime_variance = 1.0f;
    const float particle_color_variance = 0.08f;
    const float G = 0.2f;
    int spawncounter, spawnrate = 10; //todo make independent of framerate

    ParticleContainer2D particlesBrick{200};
    const float particle_brick_lifetime = 2.0f;
    const float particle_brick_lifetime_variance = 0.5f;
    const float nBrickParticles = 20;

    //For post processing
    uint framebuffers[2];
    uint colorTextures[2];

    Shader blurShader{"shader/PostProcess.vs", "shader/Blur.fs"};
    Shader textureDraw{"shader/PostProcess.vs", "shader/PostProcessRenderToScreen.fs"};

#pragma endregion

    void ComputeVertices()
    {
        vertices[0] = {0, 0};
        for (uint i = 1; i <= 32; i++)
        {
            vertices[i] = {bradius * cos(i * 2 * M_PI / 32),
                           bradius * sin(i * 2 * M_PI / 32)};
        }
        vertices[32 + 1] = {bradius * cos(2 * M_PI / 32),
                            bradius * sin(2 * M_PI / 32)};
    }

    void DragMouse()
    {
        glm::dvec2 mpos;

        glfwGetCursorPos(loader->GetWindow(), &mpos.x, &mpos.y);

        barx = mpos.x / loader->GetWindow().GetWidth() * 2 * bounds.x - bounds.x;
    }

    void UpdateBuffer()
    {
        instance_buffer.Bind(GL_ARRAY_BUFFER);

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(BufferElement) * boxes.size(), boxes.data());

        Buffer::Unbind(GL_ARRAY_BUFFER);
    }

    //Possible optimization: store rows in the buffer and compute final position in shader
    //I could also try computing collisions in the shader, but that could cause threading issues
    void KillParticles(Particle2D &p)
    {
        p.life *= p.position.y > -1.0f;
        //Figure out which box the particle is in
        float collumn = roundf((p.position.x + 1.0f - box_size.x / 2.0f) / box_size.x);
        float row = roundf((p.position.y - box_size.y / 2.0f) / box_size.y);
        if (row >= 0 && row < rows && collumn >= 0 && collumn < cols)
        {
            for (auto &b : boxes)
            {
                float bcol = roundf((b.pos.x + 1.0f - box_size.x / 2.0f) / box_size.x);
                float brow = roundf((b.pos.y - box_size.y / 2.0f) / box_size.y);
                if (bcol == collumn && brow == row)
                {
                    p.life = 0;
                    break;
                }
            }
        }
    }

    std::function<void(Particle2D &)> BoundKillParticles = std::bind(&Breakout::KillParticles, this, std::placeholders::_1);

    void RenderParticles()
    {
        particles.FindAliveParticles();
        particles.UpdateParticles(loader->GetTimeInfo().RenderDeltaTime());
        particles.ApplyFunction([this](Particle2D &p)
                                { p.velocity.y -= G * loader->GetTimeInfo().RenderDeltaTime(); });
        particles.ApplyFunction(BoundKillParticles);

        particlesBrick.FindAliveParticles();
        particlesBrick.UpdateParticles(loader->GetTimeInfo().RenderDeltaTime());
        particlesBrick.ApplyFunction([this](Particle2D &p)
                                     { p.velocity.y -= G * loader->GetTimeInfo().RenderDeltaTime(); });
        particlesBrick.ApplyFunction(BoundKillParticles);

        if (spawncounter++ % spawnrate == 0)
            particles.Emit({b_pos, 0},
                           (particle_speed_avg + particle_speed_dev * (rand() / (float)RAND_MAX * 2 - 1)) * glm::normalize(glm::vec2(rand() / (float)RAND_MAX * 2 - 1, rand() / (float)RAND_MAX * 2 - 1)),
                           b_color + particle_color_variance * glm::vec4(glm::normalize(glm::vec3(rand() / (float)RAND_MAX * 2 - 1, rand() / (float)RAND_MAX * 2 - 1, rand() / (float)RAND_MAX * 2 - 1)), 1.0f),
                           particle_size + particle_size_variance * (rand() / (float)RAND_MAX * 2 - 1), particle_lifetime + particle_lifetime_variance * (rand() / (float)RAND_MAX * 2 - 1));

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        particles.Render(ortho);
        particlesBrick.Render(ortho);
        
        //Now render the particles to the screen as is.
        //I did not want to modify the particle shader to output to different buffers, so I just draw this texture before post processing

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        va.Bind();
        glBindTexture(GL_TEXTURE_2D, colorTextures[0]);
        textureDraw.Bind();
        glDrawArrays(GL_TRIANGLES, 34, 6);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1]);
        glClear(GL_COLOR_BUFFER_BIT);
        va.Bind();
        glBindTexture(GL_TEXTURE_2D, colorTextures[0]);
        blurShader.Bind();
        blurShader.SetUniform1i("u_vertical", 0);
        //Note to future self. DO NOT forget the .0f it took me 5 days to find this error and I really started to question my sanity
        blurShader.SetUniform1f("u_offset", 1.0 / loader->GetWindow().GetWidth());
        glDrawArrays(GL_TRIANGLES, 34, 6);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, colorTextures[1]);
        blurShader.SetUniform1i("u_vertical", 1);
        blurShader.SetUniform1f("u_offset", 1.0 / loader->GetWindow().GetHeigth());
        glDrawArrays(GL_TRIANGLES, 34, 6);

        auto &color = loader->GetWindow().bgcolor;

        glClearColor(color.r, color.g, color.b, color.a);
    }

    void Render()
    {
        DragMouse();

        UpdateBuffer();

        shader.Bind();

        shader.SetUniform4f("u_Color", b_color);
        shader.SetUniformMat4f("u_MVP", ortho * glm::translate(glm::vec3(b_pos, 0.2)));

        va.Bind();

        glDrawArrays(GL_TRIANGLE_FAN, 0, 32 + 2);

        shader.SetUniform4f("u_Color", {1, 0, 0, 1});
        //Needed because opengl seems to draw only once if something already is at this z value
        shader.SetUniformMat4f("u_MVP", ortho * glm::scale(glm::translate(glm::vec3(barx, bary, 0.1)), glm::vec3(centerLineWidth, bar_size.y, 0)));
        glDrawArrays(GL_TRIANGLES, 34, 6);

        shader.SetUniform4f("u_Color", glm::vec4(1, 1, 1, 1));
        shader.SetUniformMat4f("u_MVP", ortho * glm::scale(glm::translate(glm::vec3(barx, bary, 0.01)), glm::vec3(bar_size, 0)));

        glDrawArrays(GL_TRIANGLES, 34, 6);

        bshader.Bind();

        bshader.SetUniformMat4f("u_MVP", ortho);
        bshader.SetUniformMat4f("scale_mat", glm::scale(glm::vec3(0.5f * box_size, -0.1f)));

        glDrawArraysInstanced(GL_TRIANGLES, 34, 6, boxes.size());

        bgshader.Bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, BG);

        glDrawArrays(GL_TRIANGLES, 34, 6);

        bgshader.UnBind();

        RenderParticles();

        Shader::UnBind();
        va.Unbind();
    }

    void BounceWalls()
    {
        for (int i = 0; i < 2; i++)
        {
            if (b_pos[i] + bradius > bounds[i] || b_pos[i] - bradius < -bounds[i])
            {
                b_vel[i] = -b_vel[i];
                points = 0;
            }
        }
    }

    void CollideBar()
    {
        if (b_pos.y - bradius < bary + bar_size.y && b_pos.x < barx + bar_size.x && b_pos.x > barx - bar_size.x)
        {
            float offset = (b_pos.x - barx) / bar_size.x;
            b_vel = glm::rotate(glm::vec2(0, glm::length(b_vel) + speedincrease), glm::radians(-offset * 60));
            b_pos.y = bary + bar_size.y + bradius;
        }
    }

    void UpdatePosition(float dt)
    {
        b_pos += b_vel * dt;
    }

    void CollideBottom()
    {
        if (b_pos.y - bradius < -bounds.y)
        {
            Setup();
        }
    }

    bool circleRectCollision(glm::vec2 circlepos, float radius, glm::vec2 rectpos, glm::vec2 rectsize)
    {
        float testX = circlepos.x;
        float testY = circlepos.y;

        if (circlepos.x < rectpos.x)
            testX = rectpos.x; // test left edge
        else if (circlepos.x > rectpos.x + rectsize.x)
            testX = rectpos.x + rectsize.x; // right edge
        if (circlepos.y < rectpos.y)
            testY = rectpos.y; // top edge
        else if (circlepos.y > rectpos.y + rectsize.y)
            testY = rectpos.y + rectsize.y; // bottom edge

        float distX = circlepos.x - testX;
        float distY = circlepos.y - testY;
        float distance = sqrt((distX * distX) + (distY * distY));

        if (distance <= radius)
        {
            return true;
        }
        return false;
    }

    void CollideBoxes()
    {
        for (int j = boxes.size() - 1; j >= 0; j--)
        {
            auto &box = boxes[j];
            if (circleRectCollision(b_pos, bradius, box.pos - box_size / 2.0f, box_size))
            {
                for (int i = 0; i < b_pos.length(); i++)
                {
                    //TODO fix corners
                    if (b_pos[i] > box.pos[i] + box_size[i] / 2.0f)
                    {
                        b_vel[i] = -b_vel[i];
                    }
                    else if (b_pos[i] < box.pos[i] - box_size[i] / 2.0f)
                    {
                        b_vel[i] = -b_vel[i];
                    }
                }
                //This works, but I really need to make the particles thread safe
                for (uint i = 0; i < nBrickParticles; i++)
                {
                    particles.Emit({box.pos, 0},
                                   (particle_speed_avg + particle_speed_dev * (rand() / (float)RAND_MAX * 2 - 1)) * glm::normalize(glm::vec2(rand() / (float)RAND_MAX * 2 - 1, rand() / (float)RAND_MAX * 2 - 1)),
                                   box.color + particle_color_variance * glm::vec4(glm::normalize(glm::vec3(rand() / (float)RAND_MAX * 2 - 1, rand() / (float)RAND_MAX * 2 - 1, rand() / (float)RAND_MAX * 2 - 1)), 1.0f),
                                   particle_size + particle_size_variance * (rand() / (float)RAND_MAX * 2 - 1),
                                   particle_brick_lifetime + particle_brick_lifetime_variance * (rand() / (float)RAND_MAX * 2 - 1));
                }
                boxes.erase(boxes.begin() + j);
            }
        }
    }

    void Fly()
    {
        auto dt = loader->GetTimeInfo().UpdateInterval();

        CollideBottom();
        BounceWalls();
        CollideBar();
        CollideBoxes();

        UpdatePosition(dt);
    }

    void Setup()
    {
        particles.Clear();
        uint seed = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        srand(seed);
        boxes.clear();
        for (uint i = 0; i < rows; i++)
            for (uint j = 0; j < cols; j++)
            {
                glm::vec2 pos{-1 + j * box_size.x + box_size.x / 2.0f, i * box_size.y + box_size.y / 2.0f};
                glm::vec4 color{rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1};
                boxes.push_back({pos, color});
            }
        b_pos = {0, -0.3};
        b_vel = {rand() / (float)RAND_MAX * 2 - 1, rand() / (float)RAND_MAX / 4 - 1};
        b_color = {rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, 1};
    }

    void SetupBackground()
    {
        glGenTextures(1, &BG);
        glBindTexture(GL_TEXTURE_2D, BG);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width, height, channels;
        void *data = stbi_load("res/Textures/background.png", &width, &height, &channels, STBI_rgb_alpha);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);

        bgshader.Bind();
        bgshader.SetUniform1i("u_texture", 0);
        bgshader.SetUniform1f("u_zOffset", 0.01f);
        //TODO: use 3d matrices
        //TODO: fix rotation and size
        glm::mat4 scalemat = glm::scale(glm::vec3(loader->GetWindow().GetWidth() / (float)width / 2, loader->GetWindow().GetHeigth() / (float)height / 2, 1));
        bgshader.SetUniformMat4f("u_MVP", scalemat);
    }

    void ComputeKernel()
    {
        float sigma = 4.0f;
        const uint kernelSize = 25;
        float kernel[kernelSize];
        float sum = 0;
        for (int i = 0; i < kernelSize; i++)
        {
            int offset = abs(long(i - kernelSize / 2));
            kernel[i] = exp(-offset * offset / (2 * sigma * sigma));
            sum += kernel[i];
        }
        for (uint i = 0; i < kernelSize; i++)
        {
            kernel[i] /= sum;
        }
        blurShader.SetUniform1fv("u_kernel", kernel, kernelSize);
    }

    void SetupBlur()
    {
        glGenFramebuffers(2, framebuffers);
        glGenTextures(2, colorTextures);

        for (uint i = 0; i < 2; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
            glBindTexture(GL_TEXTURE_2D, colorTextures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, loader->GetWindow().GetWidth(), loader->GetWindow().GetHeigth(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTextures[i], 0);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        blurShader.Bind();
        blurShader.SetUniform1i("u_texture", 0);
        ComputeKernel();

        textureDraw.Bind();
        textureDraw.SetUniform1f("u_zOffset",+0.008f);
        textureDraw.SetUniform1i("u_texture", 0);

        Shader::UnBind();
    }

public:
    Breakout(SceneLoader *_loader) : Scene(_loader)
    {

#pragma region Buffers
        va.Bind();

        vb.Bind(GL_ARRAY_BUFFER);

        VertexBufferLayout layout;
        layout.Push(GL_FLOAT, 2, 0);
        layout.AddToVertexArray(va);

        ComputeVertices();

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(square_vertices), 0, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(square_vertices), square_vertices);

        instance_buffer.Bind(GL_ARRAY_BUFFER);
        glBufferData(GL_ARRAY_BUFFER, sizeof(BufferElement) * rows * cols, 0, GL_DYNAMIC_DRAW);

        VertexBufferLayout instance_layout;
        instance_layout.stride = sizeof(BufferElement);
        instance_layout.Push(GL_FLOAT, 2, 0);
        instance_layout.Push(GL_FLOAT, 4, sizeof(glm::vec2));
        instance_layout.attribdivisor = 1;
        instance_layout.AddToVertexArray(va);

        Buffer::Unbind(GL_ARRAY_BUFFER);
        VertexArray::Unbind();

        SetupBlur();

#pragma endregion

        Setup();

        RegisterFunc(CallbackType::Render, &Breakout::Render, this);
        RegisterFunc(CallbackType::Update, &Breakout::Fly, this);

        auto &window = loader->GetWindow();

        if (window.GetWidth() > window.GetHeigth())
        {
            ortho = glm::ortho(-(float)window.GetWidth() / window.GetHeigth(), (float)window.GetWidth() / window.GetHeigth(), -1.0f, 1.0f);
            bounds.x = (float)window.GetWidth() / window.GetHeigth();
            bounds.y = 1.0f;
        }
        else
        {
            ortho = glm::ortho(-1.0f, 1.0f, -(float)window.GetHeigth() / window.GetWidth(), (float)window.GetHeigth() / window.GetWidth());
            bounds.x = 1.0f;
            bounds.y = (float)window.GetHeigth() / window.GetWidth();
        }

        GetFlag("hide_menu") = true;

        loader->GetWindow().bgcolor = {0.0f, 0.0f, 0.0f, 1.0f};
        SetupBackground();
    }
    ~Breakout()
    {
        loader->GetWindow().bgcolor = Window::defaultbg;
        glDeleteTextures(2, colorTextures);
        glDeleteFramebuffers(2, framebuffers);
    }
};

SCENE_LOAD_FUNC(Breakout)