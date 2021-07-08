#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <Logger.hpp>
#include <regex>
#include <unordered_map>
namespace GL::Voxel
{
    /**
     * @brief This class reads a file and stores the correct textures for each block face.
     */
    class TexConfig
    {
        mutable Logger log;
        struct Element
        {
            std::string name;

            union
            {
                struct
                {
                    uint front, back, bottom, top, right, left;
                };
                uint faces[6];
            };
        };
        std::vector<Element> elements;
        mutable std::unordered_map<std::string, uint> elements_cache;

    public:

        /**
         * @brief This will load the config. The texture itself doesn't have to be loaded.
         */
        TexConfig(const std::string &path)
        {
            std::ifstream file(path);
            std::string line;
            Element default_tex;//Will be used if a non-existing texture is requested. This should be the bottom-most texture in the image file.
            default_tex.name = "default";
            for (uint i = 0; i < 6; i++)
            {
                default_tex.faces[i] = 0;
            }
            elements.push_back(default_tex);

            std::regex rgx("^\\w(\\d|\\w)*:(\\d+,){5}\\d+(\\s*#.*)*$");
            std::regex ignore("^\\s*(#.*)?$");//Ignore comments
            std::regex name("^\\w(\\d|\\w)*:");
            while (std::getline(file, line))
            {
                //If the line is a comment, ignore it.
                if (std::regex_match(line, ignore))
                    continue;

                Element e;
                //Check if all the faces and the name are present.
                if (!std::regex_match(line, rgx))
                {
                    log << "Invalid Block " << line << " ignoring line\n";
                    continue;
                }

                //The name is everything before the first ":".
                e.name = line.substr(0, line.find(':'));
                //Remove the name from the line.
                line = line.substr(line.find(':') + 1);
                //Loop over the rest of the string and save the indices of the textures.
                for (int i = 0; i < 5; i++)
                {
                    e.faces[i] = std::stoi(line.substr(0, line.find(',')));
                    line = line.substr(line.find(',') + 1);
                }
                //Everything after an # is ignored.
                e.faces[5] = std::stoi(line.substr(0, line.find('#'))); //Last element, there is no comma.
                elements.emplace_back(std::move(e));

                //Print all errors. The false means there will be no newline.
                log.print(false);
            }
        }

        ///@brief If provided with a string this function will try to find the corresponding textures. If no match is found, the default(0) will be used.
        uint FindByName(const std::string &name) const
        {
            if (auto it = elements_cache.find(name); it != elements_cache.end())
                return it->second;
            auto pos = std::find_if(elements.begin(), elements.end(), [&](const Element &e) -> bool { return e.name == name; });
            uint index;
            if (pos == elements.end())
            {
                index = 0;
                log << "Block " << name << " not found. Using default";
                log.print();
            }
            else 
            {
                index=pos - elements.begin();
            }
            elements_cache[name]=index;
            return index;
        }

        /**
         * @brief After retrieving the index of the block, it is easier to access it directly. This also removes the need for string comparisons.
         * 
         * @param pos The index returned by FindByName.
         * @return const Element& 
         */
        const Element &operator[](uint pos) const
        {
            //If an invalid index is provided, return the default texture.
            if (pos >= elements.size())
            {
                return elements[0];
            }
            return elements[pos];
        }
    };
} // namespace GL
