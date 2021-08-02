/**
 * @file Commonfs.hpp
 * @brief Common loading and saving operations.
 */

#pragma once

#include <stdio.h>
#include <vector>
#include <string>
#include <Logger.hpp>

namespace GL::Voxel
{
    /**
     * @brief This class describes what data is stored in a file.
     * 
     * It also reads and writes the data from the disk. I created this to store the player and chunk data in my voxel game.
     */
    class FileLayout
    {
        struct IElement
        {
            virtual void Store(FILE *) = 0;
            virtual void Load(FILE *) = 0;
            virtual ~IElement() {}

        protected:
            IElement() = default;
        };
        std::vector<IElement *> elements;
        std::string fname;

    public:
        template <typename T>
        struct Element : IElement
        {
            T *data;
            unsigned int n; //< How many objects should be stored.
            void Store(FILE *f) override
            {
                fwrite(data, sizeof(T), n, f);
            }
            void Load(FILE *f) override
            {
                fread(data, sizeof(T), n, f);
            }
            Element(T *d, unsigned int nn) : data(d), n(nn) {}
            Element(const Element&)=delete;
            Element& operator=(const Element&)=delete;
        };

        //Same as Element, but both size and data just pointers, so they can be changed afterwards.
        template <typename T>
        struct Element_ptr : IElement
        {
            T *data;
            unsigned int *n;
            void Store(FILE *f) override
            {
                fwrite(data, sizeof(T), *n, f);
            }
            void Load(FILE *f) override
            {
                fread(data, sizeof(T), *n, f);
            }
            Element_ptr(T *d, unsigned int *nn) : data(d), n(nn) {}

            Element_ptr(const Element_ptr&)=delete;
            Element_ptr& operator=(const Element_ptr&)=delete;
        };
        bool Store()
        {
            auto f = fopen(fname.c_str(), "w");
            if (!f)
            {
                GL::Logger()("Could not open/create file " + fname);
                return 0;
            }
            for (IElement *x : elements)
                x->Store(f);
            fclose(f);
            return 1;
        }
        bool Load()
        {
            auto f = fopen(fname.c_str(), "r");
            if (!f)
                return 0;
            for (IElement *x : elements)
                x->Load(f);
            fclose(f);
            return 1;
        }
        template <typename T>
        void AddElement(T *data, unsigned int nn = 1)
        {
            elements.emplace_back(new Element<T>(data, nn));
        }

        /**
         * @brief T should be a pointer to a pointer because it is supposed to be changed.
         */
        template <typename T>
        void AddElement(T *data, unsigned int *nn)
        {
            elements.emplace_back(new Element_ptr<T>(data, nn));
        }
        FileLayout(const std::string &name) : fname(name) {}
        FileLayout(std::string &&name) : fname(std::move(name)) {}
        ~FileLayout()
        {
            for (IElement *e : elements)
                delete e;
        }

        FileLayout(const FileLayout &)=delete;
        FileLayout& operator=(const FileLayout &)=delete;
    };
}