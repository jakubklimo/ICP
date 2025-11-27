#pragma once
#include <string>
#include <GL/glew.h>

class Texture {
public:
	Texture(const std::string& path);
	~Texture();

	void bind() const;
	void unbind() const;

private:
	GLuint textureID;
	int width, height;
};
