///@file
#include <Data.hpp>
#include <filesystem>

std::string ROOT_Directory = std::filesystem::exists("./res") ? "." : ROOT_DIR;
