#include "World.h"

void generateChunkMesh(const World &world, const ChunkPos &chunkPos, Mesh &mesh) {
	const Chunk *const chunk = world.getChunk(chunkPos);
	// if(chunk == nullptr)
	// 	return;

	const Chunk *const xMinus = world.getChunk({chunkPos.x()-1, chunkPos.y(),   chunkPos.z()  });
	const Chunk *const xPlus  = world.getChunk({chunkPos.x()+1, chunkPos.y(),   chunkPos.z()  });
	const Chunk *const yMinus = world.getChunk({chunkPos.x(),   chunkPos.y()-1, chunkPos.z()  });
	const Chunk *const yPlus  = world.getChunk({chunkPos.x(),   chunkPos.y()+1, chunkPos.z()  });
	const Chunk *const zMinus = world.getChunk({chunkPos.x(),   chunkPos.y(),   chunkPos.z()-1});
	const Chunk *const zPlus  = world.getChunk({chunkPos.x(),   chunkPos.y(),   chunkPos.z()+1});

	// using BlockArray = Block[16][16][16];
	// const BlockArray &blocks = chunk->blocks;
	const Chunk::BlockArray &blocks = chunk->getBlocks();

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
						const ivec3 &dir = DIRECTIONS[d];
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
							if(xMinus->getBlocks()[next.x+16][next.y][next.z].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}

						if(next.x > 15) {
							if(xPlus == nullptr) {
								// faces.push_back({ {x, y, z}, d, });
								continue;
							}
							if(xPlus->getBlocks()[next.x-16][next.y][next.z].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}

						if(next.y < 0) {
							if(yMinus == nullptr) {
								// faces.push_back({ {x, y, z}, d, });
								continue;
							}
							if(yMinus->getBlocks()[next.x][next.y+16][next.z].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}

						if(next.y > 15) {
							if(yPlus == nullptr) {
								// faces.push_back({ {x, y, z}, d, });
								continue;
							}
							if(yPlus->getBlocks()[next.x][next.y-16][next.z].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}

						if(next.z < 0) {
							if(zMinus == nullptr) {
								// faces.push_back({ {x, y, z}, d, });
								continue;
							}
							if(zMinus->getBlocks()[next.x][next.y][next.z+16].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}

						if(next.z > 15) {
							if(zPlus == nullptr) {
								// faces.push_back({ {x, y, z}, d, });
								continue;
							}
							if(zPlus->getBlocks()[next.x][next.y][next.z-16].type == BlockType::AIR)
								faces.push_back({ {x, y, z}, d, });
							continue;
						}
					}
				}
			}
		}
	}

	constexpr int FLOATS_PER_VERTEX = 8 + 3;

	delete[] mesh.vertices;
	delete[] mesh.indices;

	mesh.numVertices = faces.size() * 4;
	mesh.numIndices = faces.size() * 6;
	mesh.vertices = new GLfloat[mesh.numVertices * FLOATS_PER_VERTEX];
	mesh.indices = new GLuint[mesh.numIndices];

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

		const glm::vec3 &normal = DIRECTIONS[i];
		const glm::vec3 &right = RIGHT[i];
		const glm::vec3 down = glm::cross(normal, right) * -1.f;

		for(int vert = 0; vert < 4; vert++) {
			const int vertIndex = f * 4 + vert;

			glm::vec3 vertPos = glm::vec3(face.block.x*1.f, face.block.y*1.f, face.block.z*1.f) + glm::vec3(.5f, .5f, .5f);// + glm::vec3(chunkPosX*16.f, 0.f, chunkPosZ*16.f);
			vertPos += normal * .5f;
			vertPos += right * (vert%2 - .5f);
			vertPos += down * (vert/2 - .5f);

			mesh.vertices[vertIndex * FLOATS_PER_VERTEX + 0] = vertPos.x; // x
			mesh.vertices[vertIndex * FLOATS_PER_VERTEX + 1] = vertPos.y; // y
			mesh.vertices[vertIndex * FLOATS_PER_VERTEX + 2] = vertPos.z; // z
			mesh.vertices[vertIndex * FLOATS_PER_VERTEX + 3] = 1. * (vert % 2);
			mesh.vertices[vertIndex * FLOATS_PER_VERTEX + 4] = 1. * (vert / 2);
			mesh.vertices[vertIndex * FLOATS_PER_VERTEX + 5] = normal.x;
			mesh.vertices[vertIndex * FLOATS_PER_VERTEX + 6] = normal.y;
			mesh.vertices[vertIndex * FLOATS_PER_VERTEX + 7] = normal.z;

			mesh.vertices[vertIndex * FLOATS_PER_VERTEX +  8] = right.x;
			mesh.vertices[vertIndex * FLOATS_PER_VERTEX +  9] = right.y;
			mesh.vertices[vertIndex * FLOATS_PER_VERTEX + 10] = right.z;
		}

		mesh.indices[f * 6 + 0] = f * 4 + 0;
		mesh.indices[f * 6 + 1] = f * 4 + 2;
		mesh.indices[f * 6 + 2] = f * 4 + 1;
		mesh.indices[f * 6 + 3] = f * 4 + 2;
		mesh.indices[f * 6 + 4] = f * 4 + 3;
		mesh.indices[f * 6 + 5] = f * 4 + 1;
	}
}