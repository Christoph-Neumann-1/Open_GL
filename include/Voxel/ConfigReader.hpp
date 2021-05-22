#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <Logger.hpp>
#include <regex>
#include <unordered_map>
namespace GL::Voxel
{
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
        TexConfig(const std::string &path)
        {
            std::ifstream file(path);
            std::string line;
            Element default_tex;
            default_tex.name = "default";
            for (uint i = 0; i < 6; i++)
            {
                default_tex.faces[i] = 0;
            }
            elements.push_back(default_tex);

            std::regex rgx("^\\w(\\d|\\w)*:(\\d+,){5}\\d+(\\s*#.*)*$");
            std::regex ignore("^\\s*(#.*)?$");
            std::regex name("^\\w(\\d|\\w)*:");
            while (std::getline(file, line))
            {
                if (std::regex_match(line, ignore))
                    continue;

                Element e;
                if (!std::regex_match(line, rgx))
                {
                    log << "Invalid Block " << line << " ignoring line\n";
                    continue;
                }

                e.name = line.substr(0, line.find(':'));
                line = line.substr(line.find(':') + 1);
                for (int i = 0; i < 5; i++)
                {
                    e.faces[i] = std::stoi(line.substr(0, line.find(',')));
                    line = line.substr(line.find(',') + 1);
                }
                e.faces[5] = std::stoi(line.substr(0, line.find('#'))); //Last element there is no comma.
                elements.emplace_back(std::move(e));
                log.print(false);
            }
        }

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

        const Element &operator[](uint pos) const
        {
            if (pos >= elements.size())
            {
                return elements[0];
            }
            return elements[pos];
        }
    };
} // namespace GL
