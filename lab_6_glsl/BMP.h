#pragma once

#include <vector>
#include <fstream>
#include <iostream>


#include "gl/glew.h"


typedef union PixelInfo
{
    std::uint32_t Colour;
    struct
    {
        std::uint8_t B, G, R, A;
    };
} *PPixelInfo;

class BMP
{
private:
    std::uint32_t width, height;
    std::uint16_t BitsPerPixel;
    std::vector<std::uint8_t> Pixels;
    GLuint GLtexture;

public:
    BMP(const char* FilePath);
    BMP() {};
    std::vector<std::uint8_t> GetPixels() const { return this->Pixels; }
    std::uint32_t GetWidth() const { return this->width; }
    std::uint32_t GetHeight() const { return this->height; }
    bool HasAlphaChannel() { return BitsPerPixel == 32; }
    GLuint bindGLTexture(GLenum glTexture);
    void unbindGLTexture();
};

BMP::BMP(const char* FilePath)
{
    std::fstream hFile(FilePath, std::ios::in | std::ios::binary);
    if (!hFile.is_open()) throw std::invalid_argument("Error: File Not Found.");

    hFile.seekg(0, std::ios::end);
    std::size_t Length = hFile.tellg();
    hFile.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> FileInfo(Length);
    hFile.read(reinterpret_cast<char*>(FileInfo.data()), 54);

    if (FileInfo[0] != 'B' && FileInfo[1] != 'M')
    {
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. Bitmap Required.");
    }

    if (FileInfo[28] != 32)
    {
        hFile.close();
        throw std::invalid_argument("Error: Invalid File Format. 32 bit Image Required.");
    }

    BitsPerPixel = FileInfo[28];
    width = FileInfo[18] + (FileInfo[19] << 8);
    height = FileInfo[22] + (FileInfo[23] << 8);
    std::uint32_t PixelsOffset = FileInfo[10] + (FileInfo[11] << 8);
    std::uint32_t size = ((width * BitsPerPixel + 31) / 32) * 4 * height;
    Pixels.resize(size);

    hFile.seekg(PixelsOffset, std::ios::beg);
    hFile.read(reinterpret_cast<char*>(Pixels.data()), size);
    hFile.close();
}


GLuint BMP::bindGLTexture(GLenum glTexture) {
    glGenTextures(1, &this->GLtexture);
    glActiveTexture(glTexture);
    glBindTexture(GL_TEXTURE_2D, this->GLtexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, HasAlphaChannel() ? GL_RGBA8 : GL_RGB8, this->width, this->height);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
        HasAlphaChannel() ? GL_BGRA : GL_BGR, GL_UNSIGNED_BYTE, &Pixels[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR);

    return this->GLtexture;
}
void BMP::unbindGLTexture() {
    glDeleteTextures(1, &this->GLtexture);
}