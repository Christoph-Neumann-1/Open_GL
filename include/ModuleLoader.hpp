#pragma once

#include <dlfcn.h>
#include <map>

#define RETRIEVE(rtype, name, ...) \
    Retrieve<rtype, __VA_ARGS__>(std::string(#rtype " " #name " " #__VA_ARGS__))

namespace GL
{

    class ModuleLoader
    {
        void *loaded;
        std::map<std::string, void *> *functions;

    public:
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

        template <typename rtype, typename... param>
        rtype (*Retrieve(const std::string &sig))(param...)
        {
            auto func = functions->find(sig);
            if (func != functions->end())
            {
                return reinterpret_cast<rtype (*)(param...)>(func->second);
            }
            Logger log;
            log << "Function [" << sig << "] not found. Available Functions: \n";

            for (auto &func_sig : *functions)
            {
                log << '[' << func_sig.first << ']' << '\n';
            }
            log.print();

            throw std::runtime_error("Unknown function in Module");
        }

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