#include "Chunk.h"

#include "World.h"

#include <cmath>
#include <vector>

#include <iostream> // Debugging




Chunk::Chunk(const ChunkPos &chunkPos):
		chunkPos(chunkPos),
		mesh(),
		blocks{}, dirty(true) {
	// for(int x = 0; x < 16; x++) {
	// 	for(int y = 0; y < 256; y++) {
	// 		for(int z = 0; z < 16; z++) {
	// 			blocks[x][y][z] = { (y < 5) ? BlockType::GRASS : BlockType::AIR };
	// 			// blocks[x][y][z] = { BlockType::AIR };
	// 		}
	// 	}
	// }


	static constexpr auto getHeight = [](const int blockX, const int blockZ)->uint8_t { return 8 + (int)(3*sin((blockX + blockZ * .5) * .05) + sin(blockX * .1)); };
	for(int x = 0; x < 16; x++) {
		for(int z = 0; z < 16; z++) {
			const int absX = chunkPos.x() * 16 + x;
			const int absZ = chunkPos.z() * 16 + z;
			const int height = getHeight(absX, absZ) - chunkPos.y() * 16;
			for(int y = 0; y < std::min(16, height); y++)
				blocks[x][y][z] = { BlockType::GRASS };
		}
	}

	const GLuint VBO = mesh.getVBO();
	const GLuint EBO = mesh.getEBO();
	const GLuint VAO = mesh.getVAO();

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

	glVertexArrayVertexBuffer(VAO, 0, VBO, 0, FLOATS_PER_VERTEX * sizeof(GLfloat));
	glVertexArrayElementBuffer(VAO, EBO);

	// glNamedBufferData(VBO, 20000 * 8 * sizeof(GLfloat), nullptr, GL_DYNAMIC_DRAW); // ------  TESTING -----
	// glNamedBufferData(EBO, 20000 * sizeof(GLuint), nullptr, GL_DYNAMIC_DRAW); //      ------  TESTING -----

	// generateMesh();
}

Chunk::~Chunk() {
}

// void Chunk::setVirtualOrigin(const ChunkPos &virtualOrigin) {
// }

void Chunk::generateMesh(
		const Chunk *const xMinus, const Chunk *const xPlus,
		const Chunk *const yMinus, const Chunk *const yPlus,
		const Chunk *const zMinus, const Chunk *const zPlus) {
	const GLuint VBO = mesh.getVBO();
	const GLuint EBO = mesh.getEBO();

	struct ivec3 {
		int x, y, z;
	};

	struct Face {
		ivec3 block;
		int dir;
	};

	std::vector<Face> faces;

	static constexpr ivec3 DIRECTIONS[] {
		{ 1, 0, 0 }, { -1,  0,  0 },
		{ 0, 1, 0 }, {  0, -1,  0 },
		{ 0, 0, 1 }, {  0,  0, -1 },
	};

	for(int x = 0; x < 16; x++) {
		for(int y = 0; y < 16; y++) {
			for(int z = 0; z < 16; z++) {
				if(blocks[x][y][z].type != BlockType::AIR) {
					for(int d = 0; d < 6; d++) {
						const ivec3& dir = DIRECTIONS[d];
						const ivec3 next  = ivec3{ x + dir.x, y + dir.y, z + dir.z };

						// if neighbor inside Chunk:
						if((next.x | next.y | next.z) >= 0 && next.x<16 && next.y<16 && next.z<16) {
							if(blocks[next.x][next.y][next.z].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}


						if(next.x < 0) {
							if(xMinus == nullptr) {
								// faces.push_back({ {x, y, z}, d, });
								continue;
							}
							if(xMinus->blocks[next.x+16][next.y][next.z].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}

						if(next.x > 15) {
							if(xPlus == nullptr) {
								// faces.push_back({ {x, y, z}, d, });
								continue;
							}
							if(xPlus->blocks[next.x-16][next.y][next.z].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}

						if(next.y < 0) {
							if(yMinus == nullptr) {
								// faces.push_back({ {x, y, z}, d, });
								continue;
							}
							if(yMinus->blocks[next.x][next.y+16][next.z].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}

						if(next.y > 15) {
							if(yPlus == nullptr) {
								// faces.push_back({ {x, y, z}, d, });
								continue;
							}
							if(yPlus->blocks[next.x][next.y-16][next.z].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}

						if(next.z < 0) {
							if(zMinus == nullptr) {
								// faces.push_back({ {x, y, z}, d, });
								continue;
							}
							if(zMinus->blocks[next.x][next.y][next.z+16].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}

						if(next.z > 15) {
							if(zPlus == nullptr) {
								// faces.push_back({ {x, y, z}, d, });
								continue;
							}
							if(zPlus->blocks[next.x][next.y][next.z-16].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}
					}
				}
			}
		}
	}

	int numVertices = faces.size() * 4;
	int numIndices = faces.size() * 6;
	// printf("Chunk: %d Faces, %d Vertices, %d Indices\n", faces.size(), numVertices, numIndices);

	const int FLOATS_PER_VERTEX = 8 + 3;
	GLfloat *const vertices = new GLfloat[numVertices * FLOATS_PER_VERTEX];
	GLuint *const indices = new GLuint[numIndices];


	for(int f = 0; f < faces.size(); f++) {
		const Face &face = faces[f];
		const int i = face.dir;

		constexpr glm::vec3 DIRECTIONS[] {
			{ 1.f,  0.f,  0.f }, { -1.f,  0.f,  0.f },
			{ 0.f,  1.f,  0.f }, {  0.f, -1.f,  0.f },
			{ 0.f,  0.f,  1.f }, {  0.f,  0.f, -1.f },
		};

		constexpr glm::vec3 RIGHT[] {
			{ 0.f,  0.f, -1.f }, {  0.f,  0.f,  1.f },
			{ 1.f,  0.f,  0.f }, { -1.f,  0.f,  0.f },
			{ 1.f,  0.f,  0.f }, { -1.f,  0.f,  0.f },
		};

		// const glm::vec3 &normal = DIRECTIONS[i];
		const glm::vec3 &normal = DIRECTIONS[i];
		const glm::vec3 &right = RIGHT[i];
		const glm::vec3 down = glm::cross(normal, right) * -1.f;

		for(int vert = 0; vert < 4; vert++) {
			const int vertIndex = f * 4 + vert;

			glm::vec3 vertPos = glm::vec3(face.block.x*1.f, face.block.y*1.f, face.block.z*1.f) + glm::vec3(.5f, .5f, .5f);// + glm::vec3(chunkPosX*16.f, 0.f, chunkPosZ*16.f);
			vertPos += normal * .5f;
			vertPos += right * (vert%2 - .5f);
			vertPos += down * (vert/2 - .5f);

			vertices[vertIndex * FLOATS_PER_VERTEX + 0] = vertPos.x; // x
			vertices[vertIndex * FLOATS_PER_VERTEX + 1] = vertPos.y; // y
			vertices[vertIndex * FLOATS_PER_VERTEX + 2] = vertPos.z; // z
			vertices[vertIndex * FLOATS_PER_VERTEX + 3] = 1. * (vert % 2);
			vertices[vertIndex * FLOATS_PER_VERTEX + 4] = 1. * (vert / 2);
			vertices[vertIndex * FLOATS_PER_VERTEX + 5] = normal.x;
			vertices[vertIndex * FLOATS_PER_VERTEX + 6] = normal.y;
			vertices[vertIndex * FLOATS_PER_VERTEX + 7] = normal.z;

			vertices[vertIndex * FLOATS_PER_VERTEX +  8] = right.x;
			vertices[vertIndex * FLOATS_PER_VERTEX +  9] = right.y;
			vertices[vertIndex * FLOATS_PER_VERTEX + 10] = right.z;
		}

		indices[f * 6 + 0] = f * 4 + 0;
		indices[f * 6 + 1] = f * 4 + 2;
		indices[f * 6 + 2] = f * 4 + 1;
		indices[f * 6 + 3] = f * 4 + 2;
		indices[f * 6 + 4] = f * 4 + 3;
		indices[f * 6 + 5] = f * 4 + 1;
	}

	glNamedBufferData(VBO, numVertices * FLOATS_PER_VERTEX * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
	glNamedBufferData(EBO, numIndices * sizeof(GLuint), indices, GL_STATIC_DRAW);
	// glNamedBufferSubData(VBO, 0, numVertices * FLOATS_PER_VERTEX * sizeof(GLfloat), vertices);
	// glNamedBufferSubData(EBO, 0, numIndices * sizeof(GLuint), indices);

	mesh.numIndices = numIndices;

	delete[] vertices;
	delete[] indices;

	dirty = false;
}

void Chunk::setBlock(World &world, const uint8_t x, const uint8_t y, const uint8_t z, const Block block) {
	blocks[x][y][z] = block;
	dirty = true;
	if(x == 0) {
		Chunk *chunk = world.getChunk({chunkPos.x()-1, chunkPos.y(), chunkPos.z()});
		if(chunk != nullptr)
			chunk->dirty = true;
	}
	if(y == 0) {
		Chunk *chunk = world.getChunk({chunkPos.x(), chunkPos.y()-1, chunkPos.z()});
		if(chunk)
			chunk->dirty = true;
	}
	if(z == 0) {
		Chunk *chunk = world.getChunk({chunkPos.x(), chunkPos.y(),  chunkPos.z()-1});
		if(chunk)
			chunk->dirty = true;
	}

	if(x == 15) {
		Chunk *chunk = world.getChunk({chunkPos.x()+1, chunkPos.y(), chunkPos.z()});
		if(chunk)
			chunk->dirty = true;
	}
	if(y == 15) {
		Chunk *chunk = world.getChunk({chunkPos.x(), chunkPos.y()+1, chunkPos.z()});
		if(chunk)
			chunk->dirty = true;
	}
	if(z == 15) {
		Chunk *chunk = world.getChunk({chunkPos.x(), chunkPos.y(),  chunkPos.z()+1});
		if(chunk)
			chunk->dirty = true;
	}
}