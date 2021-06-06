#pragma once
#include <Voxel/Block.hpp>
#include <array>

namespace GL::Voxel
{
    class Inventory
    {
        std::array<uint, NBLOCKS> blocks;
        BlockTypes block = BDirt;

    public:
        void Load()
        {
            auto file = fopen((ROOT_Directory + "/res/world/Inventory").c_str(), "r");
            blocks.fill(0);
            if (file)
            {
                uint n;
                fread(&n, sizeof(uint), 1, file);
                fread(&block,sizeof(BlockTypes),1,file);
                fread(&blocks,  sizeof(uint), n, file);
                fclose(file);
            }
        }
        void Store()
        {
            auto file = fopen((ROOT_Directory + "/res/world/Inventory").c_str(), "w");
            if (file)
            {
                uint n = NBLOCKS;
                fwrite(&n,  sizeof(uint), 1, file);
                fwrite(&block,sizeof(BlockTypes),1,file);
                fwrite(&blocks,  sizeof(uint), n, file);
                fclose(file);
            }
            else
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