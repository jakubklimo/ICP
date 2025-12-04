#pragma once

#include <string>
#include <GL/glew.h>
#include <opencv2/opencv.hpp>

class Texture {
public:
    Texture(const std::string& path);
    Texture(int width, int height, int channels);

    ~Texture();

    void bind() const;
    void unbind() const;

    void updateFromMat(const cv::Mat& frame);

    GLuint getID() const { return textureID; }

private:
    GLuint textureID{ 0 };
    int width{ 0 };
    int height{ 0 };
    int channels{ 0 };
    GLenum internalFormat{ GL_RGB };
    GLenum format{ GL_RGB };
};