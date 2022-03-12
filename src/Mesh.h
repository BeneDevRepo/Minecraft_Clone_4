#pragma once

#include "OpenGL/opengl_include.h"

#include <algorithm>

struct Mesh {
	GLsizei numVertices, numIndices;
	GLfloat *vertices;
	GLuint *indices;

public:
	inline Mesh(): numVertices(0), numIndices(0), vertices(nullptr), indices(nullptr) { }
	inline ~Mesh() { delete[] vertices; delete[] indices; }

	Mesh(const Mesh&) = delete;
	Mesh &operator=(const Mesh&) = delete;

	inline Mesh(Mesh&& source) {
		numVertices = source.numVertices;
		numIndices = source.numIndices;
		vertices = source.vertices;
		indices = source.indices;
		source.numVertices = 0;
		source.numIndices = 0;
		source.vertices = nullptr;
		source.indices = nullptr;
	}

	inline Mesh &operator=(Mesh&& source) {
		if (this != &source) {
			std::swap(numVertices, source.numVertices);
			std::swap(numIndices, source.numIndices);
			std::swap(vertices, source.vertices);
			std::swap(indices, source.indices);
        }
        return *this;
	}

	inline void resize(const uint32_t floatsPerVertex, const GLsizei numVerticesNew, const GLsizei numIndicesNew) { //TODO: separate capacity from used size
		delete[] vertices;
		vertices = new GLfloat[numVerticesNew * floatsPerVertex];
		numVertices = numVerticesNew;

		delete[] indices;
		indices = new GLuint[numIndicesNew];
		numIndices = numIndicesNew;
	}
};