#pragma once

#include "OpenGL/opengl_include.h"

class Texture {
private:
	GLuint texture;
	int width, height;

public:
	Texture(const char *const filename);
	~Texture();

	Texture(const Texture&) = delete;
  	Texture &operator=(const Texture&) = delete;

	inline void bind(const GLuint textureUnit) const {
		glBindTextureUnit(textureUnit, texture);
	}
};