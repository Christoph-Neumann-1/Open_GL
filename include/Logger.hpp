#pragma once
#include <sstream>
#include <string>
#include <mutex>
#include <glm/glm.hpp>

#ifndef NDEBUG
#define DEBUG_LOG
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
        static std::mutex mutex;

    public:
        void print();
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

        Logger &operator<<(glm::vec2 vec)
        {
            stream << " Vec2: " << vec.x << ' ' << vec.y << ' ';
            return *this;
        }

        Logger &operator<<(glm::vec3 vec)
        {
            stream << " Vec3: " << vec.x << ' ' << vec.y << ' ' << vec.z << ' ';
            return *this;
        }

        template <typename T>
        void operator()(const T &message)
        {
            (*this)<<message;
            this->print();
        }
    };
}