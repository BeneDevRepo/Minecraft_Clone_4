#pragma once

#include "OpenGL/opengl_include.h"

#include "glm/glm.hpp"

class StaticMesh {
private:
	GLuint VBO, EBO, VAO;

public:
	uint32_t numIndices;

public:
	glm::mat4 model;

public:
	StaticMesh();
	~StaticMesh();

	StaticMesh(const StaticMesh&) = delete;
	StaticMesh &operator=(const StaticMesh&) = delete;

	StaticMesh(StaticMesh&& other);
	StaticMesh &operator=(StaticMesh&& other);


	static StaticMesh loadSTL(const char *const fileName);
	static StaticMesh viewportRect();
	static StaticMesh cube(const glm::vec3 size = glm::vec3(1.f)); // size = diameter

	inline void bind() const {
		glBindVertexArray(VAO);
	}

	inline void draw() const {
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
	}

	inline GLuint getVBO() const {
		return VBO;
	}

	inline GLuint getEBO() const {
		return EBO;
	}

	inline GLuint getVAO() const {
		return VAO;
	}
};