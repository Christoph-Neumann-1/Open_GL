/**
 * @file ModuleLoader.hpp
 * @deprecated Please dont use this. I don't know what I was thinking. Just make a library and link it.
 */

#pragma once

#include <dlfcn.h>
#include <map>

///@brief Purely for convenience.
#define RETRIEVE(rtype, name, ...) \
    Retrieve<rtype, __VA_ARGS__>(std::string(#rtype " " #name " " #__VA_ARGS__))

namespace GL
{

    /**
     * @brief Load and access a module.
     * 
     * Can only be loaded once.
     */
    class ModuleLoader
    {
        void *loaded;
        std::map<std::string, void *> *functions;

    public:
        /**
         * @brief Construct a new Module Loader object
         * 
         * Throws a std::runtime_error if the module is not valid.
         */
        ModuleLoader(const std::string &path)
        {
            loaded = dlopen(path.c_str(), RTLD_LAZY);
            if (!loaded)
            {
                throw std::runtime_error(std::string("Error failed at loading Module ") + dlerror());
            }
            auto load_func = reinterpret_cast<std::map<std::string, void *> *(*)()>(dlsym(loaded, "_LOAD_"));
            if (!load_func)
            {
                throw std::runtime_error(std::string("Error failed at loading Module ") + dlerror());
            }
            functions = load_func();
        }

        std::vector<std::string> GetFunctionNames()
        {
            if (!loaded)
                return std::vector<std::string>();

            std::vector<std::string> names;
            for (auto &i : *functions)
            {
                names.push_back(i.first);
            }
            return names;
        }

        /**
         * @brief Retrive function by name
         * 
         * Please use the macro.
         * 
         * Usage:
         * @code{.cpp}
         * Retrieve<int,int,int>("int add int, int");
         * @endcode 
         * 
         * @tparam rtype return type
         * @tparam param what arguments it takes
         * @param sig The name
         * @return rtype(*)(param...) a pointer to the function
         */
        template <typename rtype, typename... param>
        rtype (*Retrieve(const std::string &sig))(param...)
        {
            auto func = functions->find(sig);
            if (func != functions->end())
            {
                return reinterpret_cast<rtype (*)(param...)>(func->second);
            }
            std::string str("Function [" + sig + "] not found. Available Functions: \n");

            for (auto &func_sig : *functions)
            {
                str += '[' + func_sig.first + ']' + '\n';
            }

            throw std::runtime_error(str.c_str());
        }

        /**
         * @brief Closes the module
         * 
         * Be carful with std::function when unloading.
         * 
         */
        void UnLoad()
        {
            if (loaded)
            {
                dlclose(loaded);
                loaded = nullptr;
                functions = nullptr;
            }
        }

        ~ModuleLoader()
        {
            UnLoad();
        }
    };
}