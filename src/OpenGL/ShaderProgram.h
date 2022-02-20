#pragma once

#include "OpenGL/opengl_include.h"

class ShaderProgram {
	const GLuint shaderProgram;

public:
	ShaderProgram(const char *const computeFileName);
	ShaderProgram(const char *const vertFileName, const char *const fragFileName);
	~ShaderProgram();

	inline void bind() const {
		glUseProgram(shaderProgram);
	}

	inline void unbind() const { // useless but why not
		glUseProgram(0);
	}

	inline GLint getUniformLocation(const char *const uniformName) const {
		return glGetUniformLocation(shaderProgram, uniformName);
	}

	inline operator GLuint() const {
		return shaderProgram;
	}
};