#include "ChunkRenderer.h"

// #include "World.h"


ChunkRenderer::ChunkRenderer() {
	glCreateVertexArrays(1, &VAO);

	int FLOATS_PER_VERTEX = 0;

	// Vertex Position
	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribBinding(VAO, 0, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset
	FLOATS_PER_VERTEX += 3;

	// UV
	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribBinding(VAO, 1, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset
	FLOATS_PER_VERTEX += 2;

	// Normal
	glEnableVertexArrayAttrib(VAO, 2);
	glVertexArrayAttribBinding(VAO, 2, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(VAO, 2, 3, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset
	FLOATS_PER_VERTEX += 3;

	// Tangent:
	glEnableVertexArrayAttrib(VAO, 3);
	glVertexArrayAttribBinding(VAO, 3, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(VAO, 3, 3, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset
	FLOATS_PER_VERTEX += 3;
}

ChunkRenderer::~ChunkRenderer() {
	glDeleteVertexArrays(1, &VAO); // Ignores unknown Buffer Names and 0s
}