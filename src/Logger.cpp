#include <iostream>
#include <Logger.hpp>

std::mutex GL::Logger::mutex;

void GL::Logger::print(bool newline)
{
    std::lock_guard lock(mutex);

    std::cout << stream.str() << (newline ? "\n" :"");
    std::cout.flush();

    stream.str(""); 
}