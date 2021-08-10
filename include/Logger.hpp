/**
 * @file Logger.hpp
 * @author Christoph Neumann
 * @copyright Copyright © 2021 Christoph Neumann - MIT License
 */
#pragma once
#include <sstream>
#include <string>
#include <mutex>
#include <glm/glm.hpp>

#ifndef NDEBUG
#define DEBUG_LOG //Use this to enable and disable debug messages
#endif

namespace GL
{
    /**
     * @brief Logs output to the console.
     * 
     * This class is not thread safe but the print() function is synchronized to make sure text gets printed correctly.
     */
    class Logger
    {

        std::stringstream stream;
        static std::mutex mutex;//Makes sure only one thread can print at a time.

    public:
        /**
         * @brief Print everything.
         * 
         * This function is synchronized to avoid print statements interfering.
         */
        void print(bool newline = true);

        /**
         * @brief Adds the message to the stringstream.
         * 
         * Only works if stringstream accepts T
         * 
         * @tparam T 
         * @param message 
         * @return Logger& For chaining calls.
         */
        template <typename T>
        Logger &operator<<(const T &message)
        {
            stream << message;
            return *this;
        }

        Logger &operator<<(const char *message)
        {
            if (message)
                stream << message;
            return *this;
        }

        Logger &operator<<(const char message)
        {
            if (message)
                stream << message;
            return *this;
        }

        Logger &operator<<(char *message)
        {
            return operator<<(static_cast<const char *>(message));
        }

        /// @brief Logs a vector of any size and type.
        /// I got the idea from the length function in glm. It works pretty well so far.
        template <glm::length_t L, typename T, glm::qualifier Q>
        Logger &operator<<(glm::vec<L, T, Q> const &v)
        {
            stream << "vec" << L << ": ";
            for (int i = 0; i < L; i++)
            {
                stream << v[i] << ", ";
            }
            return *this;
        }

        /**
         * @brief Clears the stream, and prints the message followed by a newline.
         * 
         * @param message 
         */
        template <typename T>
        void operator()(const T &message)
        {
            stream.str("");
            (*this) << message;
            this->print();
        }
    };
}