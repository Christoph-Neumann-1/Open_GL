#pragma once

#include <glm/glm.hpp>
#include <array>

namespace GL::Voxel
{
    struct B_Vertex
    {
        glm::vec3 pos;
        glm::vec2 tex;

        B_Vertex(glm::vec3 p, glm::vec2 t) : pos(p), tex(t) {}
    };

    const std::array<B_Vertex, 8> bvertices{
        B_Vertex({-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}),
        B_Vertex({0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}),
        B_Vertex({0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}),
        B_Vertex({-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}),

        B_Vertex({-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}),
        B_Vertex({0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}),
        B_Vertex({-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f}),
    };

    const std::array<uint, 36> bindices{
        0, 1, 2, 2, 3, 0,
        4, 7, 6, 6, 5, 4,
        0, 4, 5, 5, 1, 0,
        3, 2, 6, 6, 7, 3,
        1, 5, 6, 6, 2, 1,
        0, 3, 7, 7, 4, 0};

    struct Block
    {

        struct Vertex
        {
            glm::vec4 color;
        };
    };
}