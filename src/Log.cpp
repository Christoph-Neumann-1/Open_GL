///@file
#include <Log.hpp>

Logger &Logger::operator<<(glm::vec2 input)
{
    stream << "vec2: " << input.x << " " << input.y;
    return *this;
}

Logger &Logger::operator<<(const char *in)
{
    if (in)
        stream << in;
    return *this;
}

Logger &Logger::operator<<(char *in)
{
    if (in)
        stream << in;
    return *this;
}