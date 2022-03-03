#pragma once

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "OpenGL/opengl_include.h"
#include "OpenGL/ShaderProgram.h"

#include <unordered_map>
#include <unordered_set>
// #include <vector>

#include "Coordinates.h"

#include "Mesh.h"
#include "GPUMesh.h"

#include "ChunkMeshGenerator.h"



class World;

class ChunkMesh {
public:
	ChunkPos sourceChunk;
	Mesh mesh;
	GPUMesh gpuMesh;

public:
	inline ChunkMesh(const ChunkPos &sourceChunk):
		sourceChunk(sourceChunk),
		mesh{},
		gpuMesh{} { }

	ChunkMesh(const ChunkMesh&) = delete;
	ChunkMesh &operator=(const ChunkMesh&) = delete;

	ChunkMesh(ChunkMesh&& source):
			sourceChunk(source.sourceChunk) {
		mesh = std::move(source.mesh);
		gpuMesh = std::move(source.gpuMesh);
	}
	ChunkMesh &operator=(ChunkMesh&& source) {
		if (this != &source) {
			sourceChunk = source.sourceChunk;
			mesh = std::move(source.mesh);
			gpuMesh = std::move(source.gpuMesh);
        }
        return *this;
	}

public:
	inline void generate(const World &world) {
		generateChunkMesh(world, sourceChunk, mesh);
		gpuMesh.upload(mesh);
	}
};

class ChunkBatch {
	enum class CONTAINER_TYPE { MESHES, BATCHES } contains;
	union {
		ChunkMesh *meshes[8];
		ChunkBatch *subBatches[8];
	};
	GPUMesh mesh;

public:
	ChunkBatch();
};

class ChunkRenderer {
	GLuint VAO;
	std::unordered_map<ChunkPos, ChunkMesh*> chunks;
	std::unordered_set<ChunkPos> dirtyChunks; // may contain coordinates of non-existing chunks

public:
	ChunkRenderer();
	~ChunkRenderer();

	inline void addChunk(const World &world, const ChunkPos &chunkPos) {
		chunks.insert({chunkPos, new ChunkMesh(chunkPos)});
		markChunkDirty(chunkPos);
		markNeighborsDirty(chunkPos);
	}

	inline void removeChunk(const ChunkPos &chunkPos) {
		chunks.erase(chunkPos);
	}

	inline void markChunkDirty(const ChunkPos &chunkPos) {
		dirtyChunks.insert(chunkPos);
	}

	inline void markNeighborsDirty(const ChunkPos &chunkPos) {
		for(int dimension = 0; dimension < 3; dimension++) {
			ChunkPos dim; dim[dimension] = 1;
			markChunkDirty(chunkPos - dim);
			markChunkDirty(chunkPos + dim);
		}
	}

	inline void regenerateChunks(const World &world) {
		for(const ChunkPos &chunkPos : dirtyChunks) {
			std::unordered_map<ChunkPos, ChunkMesh*>::iterator it = chunks.find(chunkPos);
			ChunkMesh *chunk = (it == chunks.end()) ? nullptr : it->second;
			if(chunk == nullptr) continue;
			chunk->generate(world);
		}
		dirtyChunks.clear();
	}

	inline void draw(const ChunkPos &virtualOrigin, const ShaderProgram &shader) {
		constexpr GLsizei FLOATS_PER_VERTEX = 11;
		glBindVertexArray(VAO);

		for(const auto [chunkPos, chunk] : chunks) {
			const glm::mat4 model = glm::translate(glm::mat4(1.f), (chunkPos - virtualOrigin).toVec3() * 16.f);
			glProgramUniformMatrix4fv(shader, shader.getUniformLocation("model"), 1, GL_FALSE, &model[0][0]);

			glVertexArrayVertexBuffer(VAO, 0, chunk->gpuMesh.VBO, 0, FLOATS_PER_VERTEX * sizeof(GLfloat));
			glVertexArrayElementBuffer(VAO, chunk->gpuMesh.EBO);

			glDrawElements(GL_TRIANGLES, chunk->gpuMesh.numIndices, GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(0);
	}
};