#pragma once
#include <Voxel/Block.hpp>
#include <array>
#include <Voxel/Commonfs.hpp>

namespace GL::Voxel
{

    /**
     * @brief This class stores how many block the player has, as well as the currently selected one. The class can also store the info in a file.
     * 
     */
    class Inventory
    {
        std::array<uint, NBLOCKS> blocks;
        BlockTypes block = BAir;
        FileLayout file;
        uint nBlocks;

    public:
        Inventory() : file(ROOT_Directory + "/res/world/Inventory")
        {
            file.AddElement<uint>(&nBlocks);
            file.AddElement<BlockTypes>(&block);
            file.AddElement<uint>(&blocks[0],&nBlocks);
        }
///@brief First set everything to 0, then try loading the data from the disk.
        void Load()
        {
            blocks.fill(0);
            file.Load();
        }

        /**
         * @brief Stores the current state of the inventory to the disk.
         * 
         */
        void Store()
        {
            nBlocks=NBLOCKS;
            if(!file.Store())
            {
                Logger()("Failed to write inventory.");
            }
        }

        ///@brief Increment the amount of the selected block by one.
        void Add()
        {
            blocks[block]++;
        }
        ///@brief Increment a specific block by one.
        void AddTo(uint index)
        {
            blocks[index]++;
        }
        ///@brief Decrement the amount of the selected block by one. If the amount is 0, false is returned.
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
        BlockTypes GetSelected() { return block; }

        Inventory(const Inventory&)=delete;
        Inventory& operator=(const Inventory&)=delete;
    };
}