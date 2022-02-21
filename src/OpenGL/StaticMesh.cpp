#include "StaticMesh.h"

#include <cstdio>
#include <cstdint>

#include <algorithm>

#include "glm/ext/matrix_transform.hpp"



StaticMesh::StaticMesh():
		numIndices(0), model(1.f) {
	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);
	glCreateVertexArrays(1, &VAO);
}

StaticMesh::~StaticMesh() {
	// if(VAO!=0 || VBO!=0 || EBO!=0) {
	// 	printf("###########         Deleting Mesh\n");
	// 	exit(1);
	// }
	// printf("Deleting Moved-From Mesh\n");
	glDeleteVertexArrays(1, &VAO); // Ignores unknown Buffer Names and 0s
	glDeleteBuffers(1, &EBO); // Ignores unknown Buffer Names and 0s
	glDeleteBuffers(1, &VBO); // Ignores unknown Buffer Names and 0s
}


StaticMesh::StaticMesh(StaticMesh&& other):
		VAO(other.VAO),
		VBO(other.VBO),
		EBO(other.EBO),
		numIndices(other.numIndices),
		model(other.model) {

	other.VAO = 0;
	other.VBO = 0;
	other.EBO = 0;
	other.numIndices = 0;
}

StaticMesh &StaticMesh::operator=(StaticMesh&& other) {
	if(this != &other) {
		glDeleteVertexArrays(1, &VAO); // Ignores unknown Buffer Names and 0s
		glDeleteBuffers(1, &EBO); // Ignores unknown Buffer Names and 0s
		glDeleteBuffers(1, &VBO); // Ignores unknown Buffer Names and 0s

		VAO = other.VAO;
		VBO = other.VBO;
		EBO = other.EBO;
		numIndices = other.numIndices;
		model = other.model;

		other.VAO = 0;
		other.VBO = 0;
		other.EBO = 0;
		other.numIndices = 0;
	}
	return *this;
}


StaticMesh StaticMesh::loadSTL(const char *const fileName) {
	FILE *const fp = fopen(fileName, "rb");

	if(fp == nullptr)
		printf("Error reading STL File.\n");

	fseek(fp, 80, SEEK_SET);

	uint32_t numTris;
	fread(&numTris, 4, 1, fp);

	struct __attribute__ ((packed)) Triangle {
		struct Vec3 {
			GLfloat x, y, z;
		} normal, verticies[3];
		uint16_t attrCount;
	} *triangles = new Triangle[numTris];

	fread(triangles, sizeof(Triangle), numTris, fp);

	fclose(fp);

	const int numIndices = numTris * 3;// 3 vertices/indices per triangle

	GLfloat *const vertices = new GLfloat[numIndices * 8];

	for(int tri = 0; tri < numTris; tri++) {
		Triangle &triangle = triangles[tri];

		// std::swap(triangles[tri].normal.y, triangles[tri].normal.z); // swap y and z axis
		const GLfloat yTemp = triangle.normal.y;
		triangle.normal.y = triangle.normal.z;
		triangle.normal.z = yTemp;

		for(int vert = 0; vert < 3; vert++) {
			const int vertIndex = tri * 3 + vert;

			std::swap(triangle.verticies[vert].y, triangle.verticies[vert].z); // swap y and z axis

			vertices[vertIndex * 8 + 0] = triangle.verticies[vert].x * .01;
			vertices[vertIndex * 8 + 1] = triangle.verticies[vert].y * .01;
			vertices[vertIndex * 8 + 2] = -triangle.verticies[vert].z * .01;
			vertices[vertIndex * 8 + 3] = 0.;
			vertices[vertIndex * 8 + 4] = 0.;
			vertices[vertIndex * 8 + 5] = triangle.normal.x;
			vertices[vertIndex * 8 + 6] = triangle.normal.y;
			vertices[vertIndex * 8 + 7] = -triangle.normal.z;
		}
	}

	GLuint *const indices = new GLuint[numIndices];
	for(int ind = 0; ind < numIndices; ind++)
		indices[ind] = ind;


	StaticMesh out;

	// GLuint VBO, EBO, VAO;

	// Vertex + Element Buffer Object
	// glCreateBuffers(1, &VBO);
	// glCreateBuffers(1, &EBO);

	glNamedBufferData(out.VBO, numIndices * 8 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glNamedBufferData(out.EBO, numIndices * sizeof(GLuint), indices, GL_STATIC_DRAW);


	// Vertex Array Object
	glCreateVertexArrays(1, &out.VAO);

	glEnableVertexArrayAttrib(out.VAO, 0);
	glVertexArrayAttribBinding(out.VAO, 0, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(out.VAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // VAO, attribIndex, size, type, normalized, relativeoffset

	glEnableVertexArrayAttrib(out.VAO, 1);
	glVertexArrayAttribBinding(out.VAO, 1, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(out.VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset

	glEnableVertexArrayAttrib(out.VAO, 2);
	glVertexArrayAttribBinding(out.VAO, 2, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(out.VAO, 2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset

	glVertexArrayVertexBuffer(out.VAO, 0, out.VBO, 0, 8 * sizeof(GLfloat));
	glVertexArrayElementBuffer(out.VAO, out.EBO);


	delete[] indices;
	delete[] vertices;

	delete[] triangles;


	out.numIndices = numIndices;
	// out.VBO = VBO;
	// out.EBO = EBO;
	// out.VAO = VAO;
	// return std::move(out);
	return out;
}

StaticMesh StaticMesh::viewportRect() {
	const GLfloat vertices[] {
		-1.f, -1.f,   0.0f, 0.0f,
		-1.f,  1.f,   0.0f, 1.0f,
		 1.f,  1.f,   1.0f, 1.0f,
		 1.f, -1.f,   1.0f, 0.0f,
	};

	const GLuint indices[] {
		0, 2, 1,
		0, 3, 2
	};

	StaticMesh out;
	// GLuint VBO, EBO, VAO;

	// Vertex + Element Buffer Object
	// glCreateBuffers(1, &VBO);
	// glCreateBuffers(1, &EBO);

	glNamedBufferData(out.VBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glNamedBufferData(out.EBO, sizeof(indices), indices, GL_STATIC_DRAW);

	// Vertex Array Object
	// glCreateVertexArrays(1, &VAO);

	glEnableVertexArrayAttrib(out.VAO, 0);
	glVertexArrayAttribBinding(out.VAO, 0, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(out.VAO, 0, 2, GL_FLOAT, GL_FALSE, 0); // VAO, attribIndex, size, type, normalized, relativeoffset

	glEnableVertexArrayAttrib(out.VAO, 1);
	glVertexArrayAttribBinding(out.VAO, 1, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(out.VAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset

	glVertexArrayVertexBuffer(out.VAO, 0, out.VBO, 0, 4 * sizeof(GLfloat));
	glVertexArrayElementBuffer(out.VAO, out.EBO);

	out.numIndices = sizeof(indices) / sizeof(indices[0]);
	// out.VBO = VBO;
	// out.EBO = EBO;
	// out.VAO = VAO;
	// return std::move(out);
	return out;
}


StaticMesh StaticMesh::cube(const glm::vec3 size) {
	constexpr uint32_t NUM_FACES = 6;
	const int numVertices = NUM_FACES * 4; // 6 Faces * 4 Vertices/Face
	const int numIndices = NUM_FACES * 2 * 3; // 6 Faces * 2 Triangles/Face * 3 Indices/Triangle

	constexpr glm::vec3 DIRECTIONS[] {
		{1.f, 0.f, 0.f}, {-1.f,  0.f,  0.f},
		{0.f, 1.f, 0.f}, { 0.f, -1.f,  0.f},
		{0.f, 0.f, 1.f}, { 0.f,  0.f, -1.f},
	};

	constexpr glm::vec3 RIGHT[] {
		{ 0.f,  0.f, -1.f }, {  0.f,  0.f,  1.f },
		{ 1.f,  0.f,  0.f }, { -1.f,  0.f,  0.f },
		{ 1.f,  0.f,  0.f }, { -1.f,  0.f,  0.f },
	};

	const int FLOATS_PER_VERTEX = 3 + 2 + 3 + 3; // pos, UV, Normal, Tangent

	GLfloat *const vertices = new GLfloat[numVertices * FLOATS_PER_VERTEX];
	GLuint *const indices = new GLuint[numIndices];

	for(int face = 0; face < NUM_FACES; face++) {
		const glm::vec3 &normal = DIRECTIONS[face];
		const glm::vec3 &right = RIGHT[face];
		const glm::vec3 down = glm::cross(normal, right) * -1.f;

		for(int vert = 0; vert < 4; vert++) {
			const int vertIndex = face * 4 + vert;

			glm::vec3 vertPos = normal * .5f;
			vertPos += right * (vert%2 - .5f);
			vertPos += down * (vert/2 - .5f);

			vertices[vertIndex * FLOATS_PER_VERTEX + 0] = vertPos.x * size.x; // x
			vertices[vertIndex * FLOATS_PER_VERTEX + 1] = vertPos.y * size.y; // y
			vertices[vertIndex * FLOATS_PER_VERTEX + 2] = vertPos.z * size.z; // z
			vertices[vertIndex * FLOATS_PER_VERTEX + 3] = 1. * (vert % 2);
			vertices[vertIndex * FLOATS_PER_VERTEX + 4] = 1. * (vert / 2);
			vertices[vertIndex * FLOATS_PER_VERTEX + 5] = normal.x;
			vertices[vertIndex * FLOATS_PER_VERTEX + 6] = normal.y;
			vertices[vertIndex * FLOATS_PER_VERTEX + 7] = normal.z;
			vertices[vertIndex * FLOATS_PER_VERTEX + 8] = normal.x;
			vertices[vertIndex * FLOATS_PER_VERTEX + 9] = normal.y;
			vertices[vertIndex * FLOATS_PER_VERTEX + 10] = normal.z;
		}

		indices[face * 6 + 0] = face * 4 + 0;
		indices[face * 6 + 1] = face * 4 + 2;
		indices[face * 6 + 2] = face * 4 + 1;
		indices[face * 6 + 3] = face * 4 + 2;
		indices[face * 6 + 4] = face * 4 + 3;
		indices[face * 6 + 5] = face * 4 + 1;

		// indices[face * 6 + 0] = face * 4 + 0;
		// indices[face * 6 + 1] = face * 4 + 1;
		// indices[face * 6 + 2] = face * 4 + 2;
		// indices[face * 6 + 3] = face * 4 + 2;
		// indices[face * 6 + 4] = face * 4 + 1;
		// indices[face * 6 + 5] = face * 4 + 3;
	}

	StaticMesh out;
	// GLuint VBO, EBO, VAO;

	// Vertex + Element Buffer Object
	// glCreateBuffers(1, &VBO);
	// glCreateBuffers(1, &EBO);

	glNamedBufferData(out.VBO, numVertices * FLOATS_PER_VERTEX * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glNamedBufferData(out.EBO, numIndices * sizeof(GLuint), indices, GL_STATIC_DRAW);


	// Vertex Array Object
	// glCreateVertexArrays(1, &VAO);

	glEnableVertexArrayAttrib(out.VAO, 0);
	glVertexArrayAttribBinding(out.VAO, 0, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(out.VAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // VAO, attribIndex, size, type, normalized, relativeoffset

	glEnableVertexArrayAttrib(out.VAO, 1);
	glVertexArrayAttribBinding(out.VAO, 1, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(out.VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset

	glEnableVertexArrayAttrib(out.VAO, 2);
	glVertexArrayAttribBinding(out.VAO, 2, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(out.VAO, 2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset

	glEnableVertexArrayAttrib(out.VAO, 3);
	glVertexArrayAttribBinding(out.VAO, 3, 0); // VAO, attribIndex, bindingIndex
	glVertexArrayAttribFormat(out.VAO, 3, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat)); // VAO, attribIndex, size, type, normalized, relativeoffset

	glVertexArrayVertexBuffer(out.VAO, 0, out.VBO, 0, FLOATS_PER_VERTEX * sizeof(GLfloat));
	glVertexArrayElementBuffer(out.VAO, out.EBO);


	delete[] indices;
	delete[] vertices;


	// out.model = glm::scale(glm::mat4(1.f), glm::vec3(.3));
	out.numIndices = numIndices;
	// out.VBO = VBO;
	// out.EBO = EBO;
	// out.VAO = VAO;
	// return std::move(out);
	return out;
}