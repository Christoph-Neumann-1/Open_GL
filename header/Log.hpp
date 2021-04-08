///@file
#pragma once

#include <glm/glm.hpp>
#include <sstream>
#include <iostream>

/**
 * @brief Class used to print debugging output
 */
class Logger
{
    std::stringstream stream;

public:
    /**
     * @brief Prints everything to the console and
     * clears the stream.
     */
    void print()
    {
        std::cout << stream.str() << '\n'
                  << std::flush;
        stream.str("");
    }
    /**
     * @brief Inserts the input into the internal stringstream.
     * 
     * @attention Only input Types accepted by stringstream.
     * 
     * @tparam T Type to insert
     * @param input The message
     * @return Logger& reference, can be used to chain methods.
     */
    template <typename T>
    Logger &operator<<(T input)
    {
        stream << input;
        return *this;
    }

    /**
     * @brief Outputs a vec2. 
     * 
     * example: "vec2: x y"
     * 
     */
    Logger &operator<<(glm::vec2 input);

    /**
     * @brief Checks for nullpointer as well
     */
    Logger &operator<<(const char *input);
    ///@brief without const @overload
    Logger &operator<<(char *input);
};