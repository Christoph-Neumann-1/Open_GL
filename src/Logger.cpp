#include <iostream>
#include <Logger.hpp>

std::mutex GL::Logger::mutex;

void GL::Logger::print()
{
    std::lock_guard lock(mutex);

    std::cout << stream.str() << '\n';

    stream.str(""); 
}