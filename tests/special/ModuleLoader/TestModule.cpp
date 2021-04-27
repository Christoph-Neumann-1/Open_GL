#include <Module.hpp>

void Setup()
{
    FUNCTION(int, add, int, int);
    FUNCTION(float, times3, float);
}

int add(int a, int b) { return a + b; }

float times3(float n) { return n * 3; }