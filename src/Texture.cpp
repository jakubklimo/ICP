#include "Texture.h"
#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <iostream>

// --- Původní Konstruktor (Načti ze souboru) ---
Texture::Texture(const std::string& path) {
    // Načti obrázek přes OpenCV
    cv::Mat img = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (img.empty()) {
        throw std::runtime_error("Failed to load texture: " + path);
    }

    width = img.cols;
    height = img.rows;
    channels = img.channels();

    // Převeď BGR/BGRA → RGB/RGBA a urči formáty
    if (channels == 3) {
        cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
        format = GL_RGB;
        internalFormat = GL_RGB;
    }
    else if (channels == 4) {
        cv::cvtColor(img, img, cv::COLOR_BGRA2RGBA);
        format = GL_RGBA;
        internalFormat = GL_RGBA;
    }
    else {
        throw std::runtime_error("Unsupported channel count in texture: " + path);
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Alokuj paměť a nahraj data
    glTexImage2D(GL_TEXTURE_2D, 0,
        internalFormat,
        width, height, 0,
        format,
        GL_UNSIGNED_BYTE, img.data);

    glGenerateMipmap(GL_TEXTURE_2D);

    // Nastavení filtrování
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

// --- Nový Konstruktor (Dynamická Textura pro Kameru) ---
Texture::Texture(int w, int h, int c) : width(w), height(h), channels(c) {
    if (channels == 3) {
        internalFormat = GL_RGB;
        format = GL_RGB;
    }
    else if (channels == 4) {
        internalFormat = GL_RGBA;
        format = GL_RGBA;
    }
    else {
        throw std::runtime_error("Unsupported channel count for dynamic texture initialization.");
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Alokuj paměť pro dynamickou texturu, inicializuj na NULL
    glTexImage2D(GL_TEXTURE_2D, 0,
        internalFormat,
        width, height, 0,
        format,
        GL_UNSIGNED_BYTE, NULL);

    // Nastavení filtrování pro živý feed (žádné mipmapy)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

// --- Nová Metoda (Aktualizace z Mat) ---
void Texture::updateFromMat(const cv::Mat& frame) {
    if (frame.empty() || frame.cols != width || frame.rows != height) {
        std::cerr << "Warning: Skipping texture update due to invalid frame size or emptiness." << std::endl;
        return;
    }

    // Konverze BGR na RGB, protože OpenGL očekává RGB, ale OpenCV poskytuje BGR
    cv::Mat rgbFrame;
    GLenum dataFormat = (channels == 4) ? GL_RGBA : GL_RGB;

    if (channels == 3) {
        cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
    }
    else if (channels == 4) {
        cv::cvtColor(frame, rgbFrame, cv::COLOR_BGRA2RGBA);
    }
    else {
        return; // Nepodporovaný počet kanálů
    }

    glBindTexture(GL_TEXTURE_2D, textureID);

    // Použijeme glTexSubImage2D pro efektivní aktualizaci dat
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, rgbFrame.data);

    glBindTexture(GL_TEXTURE_2D, 0);
}

// --- Původní Destruktor ---
Texture::~Texture() {
    glDeleteTextures(1, &textureID);
}

// --- Původní Bind/Unbind ---
void Texture::bind() const {
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}