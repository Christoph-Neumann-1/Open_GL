/**
 * @file Particle.hpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <Shader.hpp>
#include <VertexArray.hpp>
#include <memory>

namespace GL
{
    //TODO: allow for collisions with world and forces
    //IDEA: apply custom functions to active particles for gravity and collisions
    //TODO: make thread safe
    struct Particle2D
    {
        //vec3 because of depth testing
        glm::vec3 position;
        glm::vec2 velocity;
        glm::vec4 color;
        float size;
        float life;
    };

    class ParticleContainer2D
    {
        std::unique_ptr<Particle2D[]> particles;
        uint lastDeadParticle = 0;
        Shader shader{"shader/Particle.vs", "shader/Particle.fs"};
        VertexArray vao;
        Buffer buffer;
        const uint maxParticles = 0;
        uint activeParticles = 0;
        std::unique_ptr<uint[]> aliveParticles;
        uint nAliveParticles=0;

        Particle2D &getFreeParticle();

        struct ParticleVertex2D
        {
            glm::vec3 position;
            glm::vec4 color;
            float size;
            ParticleVertex2D(const Particle2D &particle) : position(particle.position), color(particle.color), size(particle.size) {}
        };

    public:
        ParticleContainer2D(const uint _maxParticles);
        void RenderDirect(const float deltaTime, const glm::mat4 &mvp);
        void Emit(const glm::vec3 &position, const glm::vec2 &velocity, const glm::vec4 &color, const float size, const float life);
        void Clear();
        void FindAliveParticles();
        void UpdateParticles(const float deltaTime);
        void ApplyFunction(const std::function<void(Particle2D &)> &function);
        void Render(const glm::mat4 &mvp);
    };
}