#define USE_MODULE_MACROS
#include <Module.hpp>

void Setup()
{
    FUNCTION(int, add, int, int)
}

int add(int a, int b){return a+b;}