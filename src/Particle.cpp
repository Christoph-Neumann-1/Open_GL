/**
 * @file Particle.cpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */

#include <Particle.hpp>

namespace GL
{
    ParticleContainer2D::ParticleContainer2D(const uint _maxParticles) : maxParticles(_maxParticles)
    {
        vao.Bind();
        buffer.Bind(GL_ARRAY_BUFFER);
        glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(ParticleVertex2D), nullptr, GL_DYNAMIC_DRAW);
        VertexBufferLayout layout;
        layout.Push(GL_FLOAT, 3);
        layout.Push(GL_FLOAT, 4,offsetof(ParticleVertex2D, color));
        layout.Push(GL_FLOAT, 1,offsetof(ParticleVertex2D, size));
        layout.stride = sizeof(ParticleVertex2D);
        layout.AddToVertexArray(vao);
        vao.Unbind();
        buffer.Unbind(GL_ARRAY_BUFFER);

        particles.reserve(maxParticles);
    }

    void ParticleContainer2D::Render(const float deltaTime, const glm::mat4 &mvp)
    {
        buffer.Bind(GL_ARRAY_BUFFER);
        auto nRendered = 0;
        for (auto &p : particles)
        {
            if (p.life != 0.0f)
            {
                p.life = std::clamp(p.life - deltaTime, 0.0f, MAXFLOAT);
                p.position += glm::vec3(p.velocity * deltaTime, 0.0f);
                ParticleVertex2D vertex(p);
                glBufferSubData(GL_ARRAY_BUFFER, nRendered++ * sizeof(ParticleVertex2D), sizeof(ParticleVertex2D), &vertex);
            }
        }
        buffer.Unbind(GL_ARRAY_BUFFER);

        auto points = glIsEnabled(GL_PROGRAM_POINT_SIZE);
        if (!points)
            glEnable(GL_PROGRAM_POINT_SIZE);
        vao.Bind();
        shader.Bind();
        shader.SetUniformMat4f("u_MVP", mvp);
        glDrawArrays(GL_POINTS, 0, nRendered);
        vao.Unbind();
        shader.UnBind();
        if (!points)
            glDisable(GL_PROGRAM_POINT_SIZE);
    }

    void ParticleContainer2D::Emit(const glm::vec3 &position, const glm::vec2 &velocity, const glm::vec4 &color, const float size, const float life)
    {
        auto &particle = getFreeParticle();
        particle.position = position;
        particle.velocity = velocity;
        particle.color = color;
        particle.size = size;
        particle.life = life;
    }

    void ParticleContainer2D::Clear()
    {
        activeParticles = 0;
        particles.clear();
    }

    Particle2D &ParticleContainer2D::getFreeParticle()
    {
        if (activeParticles < maxParticles)
        {
            activeParticles++;
            return particles.emplace_back();
        }
        else
        {
            for (uint i = lastDeadParticle + 1; i < maxParticles; i++)
            {
                if (particles[i].life <= 0.0f)
                {
                    lastDeadParticle = i;
                    return particles[i];
                }
            }
            for (uint i = 0; i <= lastDeadParticle; i++)
            {
                if (particles[i].life <= 0.0f)
                {
                    lastDeadParticle = i;
                    return particles[i];
                }
            }
            //If no particle is available, use the one with the shortest remaining life
            float shortestRemaining = particles[0].life;
            uint index = 0;
            for (uint i = 0; i < maxParticles; i++)
            {
                if (particles[i].life < shortestRemaining)
                {
                    shortestRemaining = particles[i].life;
                    index = i;
                }
            }
            lastDeadParticle = index;
            return particles[index];
        }
    }

}