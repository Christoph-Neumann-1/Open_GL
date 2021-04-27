#pragma once

#include <string>
#include <map>
#include <initializer_list>
#include <Logger.hpp>

std::map<std::string, void *> functions;

void Setup();
extern "C" std::map<std::string, void *> *_LOAD_()
{
    Setup();
    return &functions;
}

#define FUNCTION(rtype, name, ...)  \
    rtype name(__VA_ARGS__); \
    functions[#rtype " " #name " " #__VA_ARGS__] = reinterpret_cast<void *>(name);


