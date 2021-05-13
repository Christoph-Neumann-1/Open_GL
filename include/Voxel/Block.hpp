#pragma once

#include <glm/glm.hpp>
#include <array>

namespace GL::Voxel
{
    struct B_Vertex
    {
        glm::vec3 pos;
        glm::vec2 tex;

        constexpr B_Vertex(glm::vec3 p, glm::vec2 t) : pos(p), tex(t) {}
    };

    static const std::array<B_Vertex, 36> bvertices{
        //Front
        B_Vertex({-0.5f, -0.5f, 0.5f}, {64.0f, 64.0f}),
        B_Vertex({0.5f, -0.5f, 0.5f}, {127.0f, 64.0f}),
        B_Vertex({0.5f, 0.5f, 0.5f}, {127.0f, 127.0f}),
        B_Vertex({0.5f, 0.5f, 0.5f}, {127.0f, 127.0f}),
        B_Vertex({-0.5f, 0.5f, 0.5f}, {64.0f, 127.0f}),
        B_Vertex({-0.5f, -0.5f, 0.5f}, {64.0f, 64.0f}),

        //Back
        B_Vertex({-0.5f, -0.5f, -0.5f}, {63.0f, 0.0f}),
        B_Vertex({-0.5f, 0.5f, -0.5f}, {63.0f, 63.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {0.0f, 63.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {0.0f, 63.0f}),
        B_Vertex({0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}),
        B_Vertex({-0.5f, -0.5f, -0.5f}, {63.0f, 0.0f}),

        //Bottom
        B_Vertex({-0.5f, -0.5f, 0.5f}, {64.0f, 63.0f}),
        B_Vertex({-0.5f, -0.5f, -0.5f}, {64.0f, 0.0f}),
        B_Vertex({0.5f, -0.5f, -0.5f}, {127.0f, 0.0f}),
        B_Vertex({0.5f, -0.5f, -0.5f}, {127.0f, 0.0f}),
        B_Vertex({0.5f, -0.5f, 0.5f}, {127.0f, 63.0f}),
        B_Vertex({-0.5f, -0.5f, 0.5f}, {64.0f, 63.0f}),

        //Top
        B_Vertex({-0.5f, 0.5f, 0.5f}, {64.0f, 128.0f}),
        B_Vertex({0.5f, 0.5f, 0.5f}, {127.0f, 128.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {127.0f, 191.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {127.0f, 191.0f}),
        B_Vertex({-0.5f, 0.5f, -0.5f}, {64.0f, 191.0f}),
        B_Vertex({-0.5f, 0.5f, 0.5f}, {64.0f, 128.0f}),

        //Right
        B_Vertex({0.5f, -0.5f, 0.5f}, {128.0f, 64.0f}),
        B_Vertex({0.5f, -0.5f, -0.5f}, {191.0f, 64.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {191.0f, 127.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {191.0f, 127.0f}),
        B_Vertex({0.5f, 0.5f, 0.5f}, {128.0f, 127.0f}),
        B_Vertex({0.5f, -0.5f, 0.5f}, {128.0f, 64.0f}),

        //Left
        B_Vertex({-0.5f, -0.5f, 0.5f}, {63.0f, 64.0f}),
        B_Vertex({-0.5f, 0.5f, 0.5f}, {63.0f, 127.0f}),
        B_Vertex({-0.5f, 0.5f, -0.5f}, {0.0f, 127.0f}),
        B_Vertex({-0.5f, 0.5f, -0.5f}, {0.0f, 127.0f}),
        B_Vertex({-0.5f, -0.5f, -0.5f}, {0.0f, 64.0f}),
        B_Vertex({-0.5f, -0.5f, 0.5f}, {63.0f, 64.0f})

    };
}