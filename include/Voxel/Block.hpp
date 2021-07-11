#pragma once

#include <glm/glm.hpp>
#include <array>
#include <Voxel/ConfigReader.hpp>

///Retrieves the texture information about a block with a specific name and updates the cache.
#define SET_TEX_INDEX(x) cache[B##x] = cfg.FindByName(#x)

///How many types of blocks there are
#define NBLOCKS 8

namespace GL::Voxel
{
    struct B_Vertex
    {
        glm::vec3 pos;
        glm::vec2 tex;//<Normalized texture coordinates

        constexpr B_Vertex(glm::vec3 p, glm::vec2 t) : pos(p), tex(t) {}
    };

    /**
     * @brief This is used for both lookup of information as well as data storage.
     * 
     * Air is a special case, as it is ignored. It also doesn't need a texture.
     */
    enum BlockTypes
    {
        BAir,
        BGrass,
        BDirt,
        BStone,
        BWater,
        BSand,
        BWood,
        BLeaves,
    };

    ///@brief Transparent blocks are processed differently, so you need to tell the program which block is transparent.
    bool IsTransparent(BlockTypes t)
    {
        switch (t)
        {
        case BAir:
            return true;
        case BWater:
            return true;
        case BLeaves:
            return true;
        default:
            return false;
        }
    }

    /**
     * @brief If a specific block should not be picked up, specify it here.
     */
    bool IsStorable(BlockTypes t)
    {
        switch (t)
        {
        case BWater:
            return false;
        case BAir:
            return false;

        default:
            return true;
        }
    }

    /**
     * @brief Gets the texture information for each block and returns an array for easy and fast lookup.
     * 
     * Right now every block must be added manually.
     * 
     */
    std::array<uint, NBLOCKS> MakeBlockCache(const TexConfig &cfg)
    {
        std::array<uint, NBLOCKS> cache;
        cache[BAir] = 0;
        SET_TEX_INDEX(Grass);
        SET_TEX_INDEX(Dirt);
        SET_TEX_INDEX(Stone);
        SET_TEX_INDEX(Water);
        SET_TEX_INDEX(Sand);
        SET_TEX_INDEX(Wood);
        SET_TEX_INDEX(Leaves);
        return cache;
    }

    //Normal vectors for each block face the order is the same as below
    static const std::array<glm::vec3, 6> bnormals{
        glm::vec3{0, 0, 1},
        glm::vec3{0, 0, -1},
        glm::vec3{0, -1, 0},
        glm::vec3{0, 1, 0},
        glm::vec3{1, 0, 0},
        glm::vec3{-1, 0, 0}};

    //I will have to use indices at some point but for now arrays work well enough as memory is not a problem
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