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

    static const std::array<glm::vec3,6> bnormals{
        glm::vec3{0,0,1},
        glm::vec3{0,0,-1},
        glm::vec3{0,-1,0},
        glm::vec3{0,1,0},
        glm::vec3{1,0,0},
        glm::vec3{-1,0,0}
    };

    static const std::array<B_Vertex, 36> bvertices{
        //Front
        B_Vertex({-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}),
        B_Vertex({0.5f, -0.5f, 0.5f}, {63.0f, 0.0f}),
        B_Vertex({0.5f, 0.5f, 0.5f}, {63.0f, 63.0f}),
        B_Vertex({0.5f, 0.5f, 0.5f}, {63.0f, 63.0f}),
        B_Vertex({-0.5f, 0.5f, 0.5f}, {0.0f, 63.0f}),
        B_Vertex({-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}),

        //Back
        B_Vertex({-0.5f, -0.5f, -0.5f}, {63.0f, 0.0f}),
        B_Vertex({-0.5f, 0.5f, -0.5f}, {63.0f, 63.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {0.0f, 63.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {0.0f, 63.0f}),
        B_Vertex({0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}),
        B_Vertex({-0.5f, -0.5f, -0.5f}, {63.0f, 0.0f}),

        //Bottom
        B_Vertex({-0.5f, -0.5f, 0.5f}, {0.0f, 63.0f}),
        B_Vertex({-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}),
        B_Vertex({0.5f, -0.5f, -0.5f}, {63.0f, 0.0f}),
        B_Vertex({0.5f, -0.5f, -0.5f}, {63.0f, 0.0f}),
        B_Vertex({0.5f, -0.5f, 0.5f}, {63.0f, 63.0f}),
        B_Vertex({-0.5f, -0.5f, 0.5f}, {0.0f, 63.0f}),

        //Top
        B_Vertex({-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}),
        B_Vertex({0.5f, 0.5f, 0.5f}, {63.0f, 0.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {63.0f, 63.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {63.0f, 63.0f}),
        B_Vertex({-0.5f, 0.5f, -0.5f}, {0.0f, 63.0f}),
        B_Vertex({-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f}),

        //Right
        B_Vertex({0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}),
        B_Vertex({0.5f, -0.5f, -0.5f}, {63.0f, 0.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {63.0f, 63.0f}),
        B_Vertex({0.5f, 0.5f, -0.5f}, {63.0f, 63.0f}),
        B_Vertex({0.5f, 0.5f, 0.5f}, {0.0f, 63.0f}),
        B_Vertex({0.5f, -0.5f, 0.5f}, {0.0f, 0.0f}),

        //Left
        B_Vertex({-0.5f, -0.5f, 0.5f}, {63.0f, 0.0f}),
        B_Vertex({-0.5f, 0.5f, 0.5f}, {63.0f, 63.0f}),
        B_Vertex({-0.5f, 0.5f, -0.5f}, {0.0f, 63.0f}),
        B_Vertex({-0.5f, 0.5f, -0.5f}, {0.0f, 63.0f}),
        B_Vertex({-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}),
        B_Vertex({-0.5f, -0.5f, 0.5f}, {63.0f, 0.0f})

    };
}