#include "Texture.h"
#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <iostream>

Texture::Texture(const std::string& path) {
    // Načti obrázek přes OpenCV
    cv::Mat img = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        throw std::runtime_error("Failed to load texture: " + path);
    }

        // Převeď BGR → RGB
        if (img.channels() == 3) {
            cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        }
        else if (img.channels() == 4) {
            cv::cvtColor(img, img, cv::COLOR_BGRA2RGBA);
        }

    width = img.cols;
    height = img.rows;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0,
        img.channels() == 4 ? GL_RGBA : GL_RGB,
        width, height, 0,
        img.channels() == 4 ? GL_RGBA : GL_RGB,
        GL_UNSIGNED_BYTE, img.data);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

}

Texture::~Texture() {
    glDeleteTextures(1, &textureID);
}

void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
