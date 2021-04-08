///@file
#include <glad/glad.h>

#include <Texture.hpp>
#include <Image/stb_image.h>

Texture::Texture(const std::string &path)
    : id(0), filepath(path), local_buffer(nullptr), widht(0), height(0), BPP(0), slot(0)
{
    stbi_set_flip_vertically_on_load(1);
    local_buffer = stbi_load(path.c_str(), &widht, &height, &BPP, 4);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, widht, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, local_buffer);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (local_buffer)
        stbi_image_free(local_buffer);
}

Texture::~Texture()
{
    glDeleteTextures(1, &id);
}

void Texture::Init(const std::string &path)
{
    filepath=path;
    stbi_set_flip_vertically_on_load(1);
    local_buffer = stbi_load(path.c_str(), &widht, &height, &BPP, 4);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, widht, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, local_buffer);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (local_buffer)
        stbi_image_free(local_buffer);
}

void Texture::Bind(unsigned int _slot) const
{
    this->slot = _slot;
    glActiveTexture(GL_TEXTURE0 + _slot);
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::UnBind() const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}
