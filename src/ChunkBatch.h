#pragma once

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "OpenGL/opengl_include.h"
#include "OpenGL/ShaderProgram.h"

#include "Mesh.h"
#include "GPUMesh.h"

#include "ChunkMesh.h"

#include "Coordinates.h"

#include <cstdint>

// #include "ChunkMeshGenerator.h"

class ChunkBatch {
	const ChunkPos batchPos;
	// const uint32_t level; // single chunks are stored in level 0 Batches
	const uint32_t level; // level 0 Batches represent single Chunks

	union {
		ChunkBatch *subBatches[8];
		ChunkMesh *chunks[8];
		// ChunkMesh *chunk;
	};

	Mesh mesh;
	GPUMesh gpuMesh;

public:
	bool dirty; // dirty if mesh / gpuMesh not up-to-date

public:
	static inline ChunkPos toBatchPos(const ChunkPos &chunkPos, const int32_t level) {
		constexpr auto floorDiv = [](int64_t a, int64_t b) -> int64_t {
				const bool remainder = (a % b) != 0;
				const bool resultNegative = (a < 0) != (b < 0);
				return a / b - (remainder && resultNegative);
			};

			const int32_t divisor = 1 << (level + 1); // 2 ^ (level+1)
			return {
				floorDiv(chunkPos.x(), divisor),
				floorDiv(chunkPos.y(), divisor),
				floorDiv(chunkPos.z(), divisor)
			};
	}
	static inline ChunkPos toChunkPos(const ChunkPos &batchPos, const uint32_t level) {
			return batchPos * (1 << (level + 1));
		}

public:
	inline ChunkBatch(const ChunkPos &chunkPos, const uint32_t numSubchunkLevels):
			level(numSubchunkLevels-1),
			batchPos(toBatchPos(chunkPos, numSubchunkLevels-1)), // can't use level inside initializer list
			dirty(true) {
		if(level == 0) {
			for(int i = 0; i < 8; i++)
				chunks[i] = nullptr;
		} else {
			for(int i = 0; i < 8; i++)
				subBatches[i] = nullptr;
		}
	}
	inline ~ChunkBatch() {
	}

	ChunkBatch(ChunkBatch&) = delete;
	ChunkBatch &operator=(ChunkBatch&) = delete;

	ChunkBatch(ChunkBatch&&) = delete;
	ChunkBatch &operator=(ChunkBatch&&) = delete;

public:
	inline uint8_t numSubMeshes() const {
		uint8_t num = 0;
		if(level == 0) {
			for(int i = 0; i < 8; i++)
				num += chunks[i] != nullptr;
		} else {
			for(int i = 0; i < 8; i++)
				num += subBatches[i] != nullptr;
		}
		return num;
	}

public:
	inline void addChunk(const ChunkPos &chunkPos) {
		const ChunkPos chunkPosRel = toBatchPos(chunkPos - toChunkPos(batchPos, level), level-1);

		const uint8_t index = chunkPosRel.z() * 4 + chunkPosRel.y() * 2 + chunkPosRel.x();
		if(level == 0) {
			chunks[index] = new ChunkMesh(chunkPos);
		} else {
			if(subBatches[index] == nullptr)
				subBatches[index] = new ChunkBatch(chunkPos, level);
			subBatches[index]->addChunk(chunkPos);
		}
	}

	inline void removeChunk(const ChunkPos &chunkPos) {
		const ChunkPos chunkPosRel = toBatchPos(chunkPos - toChunkPos(batchPos, level), level-1);

		const uint8_t index = chunkPosRel.z() * 4 + chunkPosRel.y() * 2 + chunkPosRel.x();
		if(level == 0) {
			delete chunks[index];
			chunks[index] = nullptr;
		} else {
			// dirty = true;
			subBatches[index]->removeChunk(chunkPos);

			if(subBatches[index]->numSubMeshes() == 0) {
				delete subBatches[index];
				subBatches[index] = nullptr;
			}
		}
	}


public:
	inline void generateCombinedMesh() {
		thread_local static Mesh *meshes[8];
		thread_local static GPUMesh *gpuMeshes[8];
		if(level == 0) {
			for(int i = 0; i < 8; i++) {
				meshes[i] = (chunks[i] == nullptr) ? nullptr : &(chunks[i]->mesh);
				gpuMeshes[i] = (chunks[i] == nullptr) ? nullptr : &(chunks[i]->gpuMesh);
			}
		} else {
			for(int i = 0; i < 8; i++) {
				meshes[i] = (subBatches[i] == nullptr) ? nullptr : &(subBatches[i]->mesh);
				gpuMeshes[i] = (subBatches[i] == nullptr) ? nullptr : &(subBatches[i]->gpuMesh);
			}
		}


		constexpr GLsizei FLOATS_PER_VERTEX = 11;

		// Generate combined Mesh:
		int numCombinedVertices = 0;
		int numCombinedIndices = 0;
		for(int i = 0; i < 8; i++) {
			if(meshes[i] == nullptr) continue; // Skip non-existing chunks
			numCombinedVertices += meshes[i]->numVertices;
			numCombinedIndices += meshes[i]->numIndices;
		}

		delete[] mesh.vertices;
		delete[] mesh.indices;
		mesh.vertices = new GLfloat[FLOATS_PER_VERTEX * numCombinedVertices];
		mesh.indices = new GLuint[numCombinedIndices];

		mesh.numVertices = 0;
		mesh.numIndices = 0;
		for(int chunk = 0; chunk < 8; chunk++) {
			if(meshes[chunk] == nullptr) continue; // Skip non-existing chunks

			for(int i = 0; i < meshes[chunk]->numIndices; i++, mesh.numIndices++)
				mesh.indices[mesh.numIndices] = meshes[chunk]->indices[i] + mesh.numVertices; // Copy (adjusted) indices

			for(int i = 0; i < meshes[chunk]->numVertices; i++, mesh.numVertices++) {
				glm::vec3 offset;
				if(level == 0)
					offset = (chunks[chunk]->sourceChunk - toChunkPos(batchPos, level)).toVec3() * 16.f;
				else
					offset = (toChunkPos(subBatches[chunk]->batchPos, level-1) - toChunkPos(batchPos, level)).toVec3() * 16.f;

				memcpy( mesh.vertices + (FLOATS_PER_VERTEX * mesh.numVertices), // Copy vertices
						meshes[chunk]->vertices + (FLOATS_PER_VERTEX * i),
						FLOATS_PER_VERTEX * sizeof(GLfloat));

				mesh.vertices[FLOATS_PER_VERTEX * mesh.numVertices + 0] += offset.x;
				mesh.vertices[FLOATS_PER_VERTEX * mesh.numVertices + 1] += offset.y;
				mesh.vertices[FLOATS_PER_VERTEX * mesh.numVertices + 2] += offset.z;
			}
		}
		dirty = false;
	}

	inline void generate(const World &world, const ChunkPos &chunkPos) {     
		const ChunkPos chunkPosRel = toBatchPos(chunkPos - toChunkPos(batchPos, level), level-1);

		const uint8_t index = chunkPosRel.z() * 4 + chunkPosRel.y() * 2 + chunkPosRel.x();
		if(level == 0) {
			if(chunks[index] != nullptr)
				chunks[index]->generate(world);
		} else {
			if(subBatches[index] != nullptr)
				subBatches[index]->generate(world, chunkPos);
		}

		dirty = true;

	}

	inline void regenerateCombinedMeshes() {
		if(!dirty) return;

		if(level > 0)
			for(int i = 0; i < 8; i++)
				if(subBatches[i] != nullptr)
					subBatches[i]->regenerateCombinedMeshes();
	
		generateCombinedMesh();
		gpuMesh.upload(mesh); // -----------------
		dirty = false;
	}

public:
	inline void draw(const ChunkPos &virtualOrigin, const ShaderProgram &shader, const GLuint VAO) {
		constexpr GLsizei FLOATS_PER_VERTEX = 11;
		if(level == 0 && dirty) {
			for(int i = 0; i < 8; i++) {
				if(chunks[i] != nullptr) {

					const glm::mat4 model = glm::translate(glm::mat4(1.f), (chunks[i]->sourceChunk - virtualOrigin).toVec3() * 16.f);
					glProgramUniformMatrix4fv(shader, shader.getUniformLocation("model"), 1, GL_FALSE, &model[0][0]);

					chunks[i]->gpuMesh.draw(VAO);

					// glVertexArrayVertexBuffer(VAO, 0, chunks[i]->gpuMesh.VBO, 0, FLOATS_PER_VERTEX * sizeof(GLfloat));
					// glVertexArrayElementBuffer(VAO, chunks[i]->gpuMesh.EBO);

					// glDrawElements(GL_TRIANGLES, chunks[i]->gpuMesh.numIndices, GL_UNSIGNED_INT, 0);
				}
			}
		} else {
			if(!dirty) {
				const glm::mat4 model = glm::translate(glm::mat4(1.f), (toChunkPos(batchPos, level) - virtualOrigin).toVec3() * 16.f);
				// const glm::mat4 model = glm::translate(glm::mat4(1.f), (batchPos - virtualOrigin).toVec3() * 16.f);
				glProgramUniformMatrix4fv(shader, shader.getUniformLocation("model"), 1, GL_FALSE, &model[0][0]);

				gpuMesh.draw(VAO);
			} else {
				for(int i = 0; i < 8; i++)
					if(subBatches[i] != nullptr)
						subBatches[i]->draw(virtualOrigin, shader, VAO);
			}
		}
	}
};