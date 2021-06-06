#pragma once
#include <Voxel/Block.hpp>
#include <array>
#include <Voxel/Commonfs.hpp>

namespace GL::Voxel
{
    class Inventory
    {
        std::array<uint, NBLOCKS> blocks;
        BlockTypes block = BDirt;
        FileLayout file;
        uint nBlocks;

    public:
        Inventory() : file(ROOT_Directory + "/res/world/Inventory")
        {
            file.AddElement<uint>(&nBlocks, 1);
            file.AddElement<BlockTypes>(&block, 1);
            file.AddElement<uint>(&blocks[0],&nBlocks);
        }

        void Load()
        {
            blocks.fill(0);
            file.Load();
        }
        void Store()
        {
            nBlocks=NBLOCKS;
            if(!file.Store())
            {
                Logger()("Failed to write inventory.");
            }
        }

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
    };
}