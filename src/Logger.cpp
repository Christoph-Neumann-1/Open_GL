/**
 * @file Logger.cpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#include <iostream>
#include <Logger.hpp>

std::mutex GL::Logger::mutex;

void GL::Logger::print(bool newline)
{
    std::lock_guard lock(mutex);

    std::cout << stream.str() << (newline ? "\n" :"");

    stream.str(""); 
}