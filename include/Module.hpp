#pragma once

#include <string>
#include <map>
#include <initializer_list>

///All functions in this file.
static std::map<std::string, void *> functions;

///@brief Makes it possible to register the function without a seperate setup function
struct FunctionInit
{
    FunctionInit(const std::string &name, void *func)
    {
        functions[name]=func;
    }
};

///@brief Called when loading.
extern "C" std::map<std::string, void *> *_LOAD_()
{
    return &functions;
}

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b

/**
 * @brief Define a new function
 * 
 * Usage:
 * @code{.cpp}
 * FUNCTION(int, add, int, int)(int a, int b){return a+b;}
 * @endcode 
 * 
 */
#define FUNCTION(rtype, name,...) \
    rtype name(__VA_ARGS__);       \
    static FunctionInit  CONCAT(name, __COUNTER__)\
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