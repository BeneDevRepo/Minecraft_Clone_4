#include "DebugRenderer.h"

DebugRenderer::DebugRenderer():
		numIndices(0), model(1.f), lines() {

	glCreateBuffers(1, &VBO);
	// glCreateBuffers(1, &EBO);
	glCreateVertexArrays(1, &VAO);


	// glNamedBufferData(VBO, numIndices * 8 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	// glNamedBufferData(EBO, numIndices * sizeof(GLuint), indices, GL_STATIC_DRAW);


	// Vertex Array Object
	glCreateVertexArrays(1, &VAO);

	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribBinding(VAO, 0, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // VAO, attribIndex, size, type, normalized, relativeoffset

	// glEnableVertexArrayAttrib(VAO, 1);
	// glVertexArrayAttribBinding(VAO, 1, 0); // VAO, attribIndex, bindingIndex
	// glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset

	// glEnableVertexArrayAttrib(VAO, 2);
	// glVertexArrayAttribBinding(VAO, 2, 0); // VAO, attribIndex, bindingIndex
	// glVertexArrayAttribFormat(VAO, 2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 3 * sizeof(GLfloat));
	// glVertexArrayElementBuffer(VAO, EBO);
}

DebugRenderer::~DebugRenderer() {
	glDeleteVertexArrays(1, &VAO); // Ignores unknown Buffer Names and 0s
	// glDeleteBuffers(1, &EBO); // Ignores unknown Buffer Names and 0s
	glDeleteBuffers(1, &VBO); // Ignores unknown Buffer Names and 0s
}

void DebugRenderer::clear() {
	lines.clear();
	numIndices = 0;
}

void DebugRenderer::line(const glm::vec3 p1, const glm::vec3 p2) {
	lines.push_back({p1, p2});
}

void DebugRenderer::line(const float x1, const float y1, const float z1, const float x2, const float y2, const float z2) {
	line({x1, y1, z1}, {x2, y2, z2});
}

void DebugRenderer::box(const glm::vec3 p1, const glm::vec3 p2) {
// p1 z plane
	line(p1.x, p1.y, p1.z, p2.x, p1.y, p1.z); // hori
	line(p1.x, p2.y, p1.z, p2.x, p2.y, p1.z); // hori
	line(p1.x, p1.y, p1.z, p1.x, p2.y, p1.z); // vert
	line(p2.x, p1.y, p1.z, p2.x, p2.y, p1.z); // vert

// p2 z plane
	line(p1.x, p1.y, p2.z, p2.x, p1.y, p2.z); // hori
	line(p1.x, p2.y, p2.z, p2.x, p2.y, p2.z); // hori
	line(p1.x, p1.y, p2.z, p1.x, p2.y, p2.z); // vert
	line(p2.x, p1.y, p2.z, p2.x, p2.y, p2.z); // vert

	line(p1.x, p1.y, p1.z, p1.x, p1.y, p2.z);
	line(p2.x, p1.y, p1.z, p2.x, p1.y, p2.z);
	line(p1.x, p2.y, p1.z, p1.x, p2.y, p2.z);
	line(p2.x, p2.y, p1.z, p2.x, p2.y, p2.z);
}