#pragma once

#include "OpenGL/opengl_include.h"

class Cubemap {
private:
	GLuint texture;
	int width, height;

public:
	Cubemap(const char *const filename);
	~Cubemap();

	inline void bind(const GLuint textureUnit) const {
		glBindTextureUnit(textureUnit, texture);
	}
};