#include <Module.hpp>

FUNCTION(int, add, int, int)
(int a, int b) { return a + b; }

FUNCTION(float, add, float, float)
(float a, float b) { return a + b; }

FUNCTION(float, times3, float)
(float n) { return n * 3; }
