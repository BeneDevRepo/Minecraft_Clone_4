#pragma once

#include "OpenGL/opengl_include.h"

#include "glm/glm.hpp"

#include <vector>

class DebugRenderer;

extern DebugRenderer *DEBUG_RENRERER;

class DebugRenderer {
private:
	GLuint VAO;
	GLuint VBO;//, EBO;
// public:
	uint32_t numIndices;

private:
	struct Line { const glm::vec3 p1, p2; };
	std::vector<Line> lines;

public:
	const glm::mat4 model;

public:
	DebugRenderer();
	~DebugRenderer();

	DebugRenderer(const DebugRenderer&) = delete;
	DebugRenderer &operator=(const DebugRenderer&) = delete;

	void clear();
	void line(const glm::vec3 p1, const glm::vec3 p2);
	void line(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2);
	void box(const glm::vec3 p1, const glm::vec3 p2);

	inline void bind() {
		glNamedBufferData(VBO, (lines.size()*2) * 3 * sizeof(GLfloat), lines.data(), GL_DYNAMIC_DRAW);
		// glNamedBufferData(EBO, numIndices * sizeof(GLuint), indices, GL_DYNAMIC_DRAW);

		glBindVertexArray(VAO);
	}

	inline void draw() const {
		glLineWidth(3.f);
		// glDrawElements(GL_LINES, numIndices, GL_UNSIGNED_INT, 0);
		glDrawArrays(GL_LINES, 0, lines.size()*2);
	}
};