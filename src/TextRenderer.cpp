#include "TextRenderer.h"

#include <string>

TextRenderer::TextRenderer(const char *const fontFolder):
			fontMeta(fontFolder),
			texture((std::string(fontFolder) + "/font.png").c_str()) {

	glCreateVertexArrays(1, &VAO);

	int FLOATS_PER_VERTEX = 0;

	// Vertex Position
	glEnableVertexArrayAttrib(VAO, 0);
	glVertexArrayAttribBinding(VAO, 0, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(VAO, 0, 2, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset
	FLOATS_PER_VERTEX += 2;

	// UV
	glEnableVertexArrayAttrib(VAO, 1);
	glVertexArrayAttribBinding(VAO, 1, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset
	FLOATS_PER_VERTEX += 2;

	// // Normal
	// glEnableVertexArrayAttrib(VAO, 2);
	// glVertexArrayAttribBinding(VAO, 2, 0); // VAO, attribIndex, bindingIndex
	// glVertexArrayAttribFormat(VAO, 2, 3, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset
	// FLOATS_PER_VERTEX += 3;

	// // Tangent:
	// glEnableVertexArrayAttrib(VAO, 3);
	// glVertexArrayAttribBinding(VAO, 3, 0); // VAO, attribIndex, bindingIndex
	// glVertexArrayAttribFormat(VAO, 3, 3, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset
	// FLOATS_PER_VERTEX += 3;

	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);

	glNamedBufferStorage(VBO, (MAX_CHARACTERS_BUFFER * 4) * (4 * sizeof(GLfloat)), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glNamedBufferStorage(EBO, (MAX_CHARACTERS_BUFFER * 6) * sizeof(GLuint), nullptr, GL_DYNAMIC_STORAGE_BIT);

	GLuint indices[] {
		0, 1, 2,
		1, 3, 2
	};
	for(int i = 0; i < MAX_CHARACTERS_BUFFER; i++) {
		glNamedBufferSubData(EBO, i * 6*sizeof(GLuint), 6*sizeof(GLuint), indices);
		for(int j = 0; j < 6; j++)
			indices[j] += 4;
	}

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 4*sizeof(GLfloat));
	glVertexArrayElementBuffer(VAO, EBO);
}

TextRenderer::~TextRenderer() {
	glDeleteVertexArrays(1, &VAO); // Ignores unknown Buffer Names and 0s
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
}