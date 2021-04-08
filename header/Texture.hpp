///@file
#pragma once

#include <string>

///@brief Handles Texture loading and preprocessing
class Texture
{
    unsigned int id;
    std::string filepath;
    unsigned char *local_buffer;
    int widht, height, BPP;
    mutable unsigned int slot;

public:
    /**
     * @brief Load the texture at path and generate a buffer.
     * 
     * @param path Where the texture is located
     */
    explicit Texture(const std::string &path);
    Texture() : id(0), local_buffer(nullptr), widht(0), height(0), BPP(0), slot(0) {}
    ~Texture();

    void Init(const std::string &path);

    void Bind(unsigned int slot = 0) const;  ///<Binds the texture to the %Texture slot
    void UnBind() const;                     ///<Unbinds the Texture at the previously defined slot. @note Unclear if it unbinds the intended Texture
    int GetWidth() const { return widht; }   ///<@return Width in pixels
    int GetHeigth() const { return height; } ///<@return Heigth in pixels
    int GetBPP() const { return BPP; }       ///< Bits per pixel
};