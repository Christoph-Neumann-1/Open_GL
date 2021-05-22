#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <Logger.hpp>
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
            while (std::getline(file, line))
            {
                line = line.substr(line.find(':') + 1);
                Element e;
                e.name = line.substr(0, line.find(':'));
                line = line.substr(line.find(':') + 1);
                for (int i = 0; i < 6; i++)
                {
                    e.faces[i] = std::stoi(line.substr(0, line.find(',')));
                    line = line.substr(line.find(',') + 1);
                }
                elements.emplace_back(std::move(e));
            }
        }

        uint FindByName(const std::string &name) const
        {
            auto pos = std::find_if(elements.begin(), elements.end(), [&](const Element &e) -> bool { return e.name == name; });
            return pos != elements.end() ? pos - elements.begin() : 0;
            log << "Block " << name << " not found. Using default";
            log.print();
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
