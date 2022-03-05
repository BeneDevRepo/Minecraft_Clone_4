#pragma once

#include "OpenGL/opengl_include.h"

#include <algorithm>

#include "Mesh.h"


struct GPUMesh {
	GLsizei numIndices;
	GLuint VBO, EBO;

public:
	inline GPUMesh(): numIndices(0) {
		glCreateBuffers(1, &VBO);
		glCreateBuffers(1, &EBO);
	}
	inline ~GPUMesh() {
		glDeleteBuffers(1, &EBO); // Ignores unknown Buffer Names and 0s
		glDeleteBuffers(1, &VBO); // Ignores unknown Buffer Names and 0s
	}

	GPUMesh(const GPUMesh&) = delete;
	GPUMesh &operator=(const GPUMesh&) = delete;

	inline GPUMesh(GPUMesh&& source):
			numIndices(source.numIndices),
			VBO(source.VBO),
			EBO(source.EBO) {
		source.numIndices = 0;
		source.VBO = 0;
		source.EBO = 0;
	}
	inline GPUMesh &operator=(GPUMesh&& source) {
		if (this != &source) {
			std::swap(numIndices, source.numIndices);
			std::swap(VBO, source.VBO);
			std::swap(EBO, source.EBO);
        }
        return *this;
	}

public:
	inline void upload(const Mesh &mesh) {
		const GLsizei FLOATS_PER_VERTEX = 11;

		glNamedBufferData(VBO, mesh.numVertices * FLOATS_PER_VERTEX * sizeof(GLfloat), mesh.vertices, GL_STATIC_DRAW);
		glNamedBufferData(EBO, mesh.numIndices * sizeof(GLuint), mesh.indices, GL_STATIC_DRAW);

		// glNamedBufferSubData(VBO, 0, numVertices * FLOATS_PER_VERTEX * sizeof(GLfloat), vertices);
		// glNamedBufferSubData(EBO, 0, numIndices * sizeof(GLuint), indices);

		numIndices = mesh.numIndices;
	}

	inline void draw(const GLuint VAO) {
		constexpr GLsizei FLOATS_PER_VERTEX = 11;

		glVertexArrayVertexBuffer(VAO, 0, this->VBO, 0, FLOATS_PER_VERTEX * sizeof(GLfloat));
		glVertexArrayElementBuffer(VAO, this->EBO);

		glDrawElements(GL_TRIANGLES, this->numIndices, GL_UNSIGNED_INT, 0);
	}
};