#pragma once
#include <Voxel/Block.hpp>
#include <array>

namespace GL::Voxel
{
    class Inventory
    {
        std::array<int, NBLOCKS> blocks;
        BlockTypes block = BDirt;

    public:
        void Add()
        {
            blocks[block]++;
        }
        void AddTo(uint index)
        {
            blocks[index]++;
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
            if (IsStorable(type))
                block = type;
        }
        uint GetCount()
        {
            return blocks[block];
        }
        BlockTypes GetSelected() { return (BlockTypes)(block); }

        Inventory() { blocks.fill(0); }
    };
}