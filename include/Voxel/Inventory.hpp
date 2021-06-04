#pragma once
#include <Voxel/Block.hpp>
#include <array>

namespace GL::Voxel
{
    class Inventory
    {
        std::array<int, 6> blocks;
        uint block;

        void Add()
        {
            blocks[block]++;
        }
        bool Remove()
        {
            if (blocks[block] <= 0)
                return false;

            blocks[block]--;
            return true;
        }
        void Select(BlockTypes type)
        {
            block = type - 1;
        }
        uint GetCount()
        {
            return blocks[block];
        }
        BlockTypes GetSelected() { return (BlockTypes)(block + 1); }
    };
}