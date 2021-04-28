#pragma once

#include <string>
#include <map>
#include <initializer_list>

std::map<std::string, void *> functions;

struct FunctionInit
{
    FunctionInit(const std::string &name, void *func)
    {
        functions[name]=func;
    }
};

extern "C" std::map<std::string, void *> *_LOAD_()
{
    return &functions;
}
#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b

#define FUNCTION(rtype, name,...) \
    rtype name(__VA_ARGS__);       \
    FunctionInit  CONCAT(name, __COUNTER__)\
    (#rtype " " #name " " #__VA_ARGS__,reinterpret_cast<void*>(static_cast<rtype(*)(__VA_ARGS__)>(name)));\
    rtype name

/**
 * @file
 * 
 * @brief Include in all modules.
 * 
 * Contains macros and functions that make up all modules.
 * 
 */