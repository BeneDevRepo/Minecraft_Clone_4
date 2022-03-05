#pragma once

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "OpenGL/opengl_include.h"
#include "OpenGL/ShaderProgram.h"

#include <unordered_map>
#include <unordered_set>

#include "Coordinates.h"

// #include "Mesh.h"
// #include "GPUMesh.h"

// #include "ChunkMesh.h"
#include "ChunkBatch.h"


class World;

class ChunkRenderer {
	GLuint VAO;

private:
	const uint32_t numLevels; // 1 means storing individual chunks in the top level ChunkBatches

	std::unordered_map<ChunkPos, ChunkBatch*> chunks; // chunkPos is position of Batch (depends on numLevels, if numLevels==1, pos is identical to actual chunkPos)
	// std::unordered_map<ChunkPos, ChunkMesh*> chunks;

	std::unordered_set<ChunkPos> dirtyChunks; // may contain coordinates of non-existing chunks

public:
	ChunkRenderer(const uint32_t numLevels);
	~ChunkRenderer();

	inline ChunkPos topLevelBatchPos(const ChunkPos &chunkPos) {
		return ChunkBatch::toBatchPos(chunkPos, numLevels-1);
	}

	inline ChunkBatch *getBatch(const ChunkPos &batchPos) {
		std::unordered_map<ChunkPos, ChunkBatch*>::iterator it = chunks.find(batchPos);
		return (it == chunks.end()) ? nullptr : it->second;
	}
	inline const ChunkBatch *getBatch(const ChunkPos &batchPos) const {
		std::unordered_map<ChunkPos, ChunkBatch*>::const_iterator it = chunks.find(batchPos);
		return (it == chunks.end()) ? nullptr : it->second;
	}

	inline void addChunk(const ChunkPos &chunkPos) {
		ChunkPos containingPatchPos = topLevelBatchPos(chunkPos);
		ChunkBatch *containingBatch = getBatch(containingPatchPos);
		if(containingBatch == nullptr) {
			containingBatch = new ChunkBatch(chunkPos, numLevels);
			chunks.insert({containingPatchPos, containingBatch});
		}
		containingBatch->addChunk(chunkPos);

		markChunkDirty(chunkPos);
		markNeighborsDirty(chunkPos);
	}

	inline void removeChunk(const ChunkPos &chunkPos) {
		ChunkPos containingBatchPos = topLevelBatchPos(chunkPos);
		ChunkBatch *containingBatch = getBatch(containingBatchPos);
		containingBatch->removeChunk(chunkPos);
		if(containingBatch->numSubMeshes() == 0) {
			chunks.erase(containingBatchPos);
			delete containingBatch;
		}
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
			const ChunkPos batchPos = topLevelBatchPos(chunkPos);
			std::unordered_map<ChunkPos, ChunkBatch*>::iterator it = chunks.find(batchPos);
			ChunkBatch *batch = (it == chunks.end()) ? nullptr : it->second;
			if(batch == nullptr) continue;
			batch->generate(world, chunkPos);

			// std::unordered_map<ChunkPos, ChunkMesh*>::iterator it = chunks.find(chunkPos);
			// ChunkMesh *chunk = (it == chunks.end()) ? nullptr : it->second;
			// if(chunk == nullptr) continue;
			// chunk->generate(world);
		}
		dirtyChunks.clear();

		for(const auto &[batchPos, chunkBatch] : chunks)
			if(chunkBatch->dirty)
				chunkBatch->regenerateCombinedMeshes();
	}

	inline void draw(const ChunkPos &virtualOrigin, const ShaderProgram &shader) {
		glBindVertexArray(VAO);

		for(const auto &[batchPos, chunkBatch] : chunks)
			chunkBatch->draw(virtualOrigin, shader, VAO);

		// constexpr GLsizei FLOATS_PER_VERTEX = 11;
		// for(const auto [chunkPos, chunk] : chunks) {
		// 	const glm::mat4 model = glm::translate(glm::mat4(1.f), (chunkPos - virtualOrigin).toVec3() * 16.f);
		// 	glProgramUniformMatrix4fv(shader, shader.getUniformLocation("model"), 1, GL_FALSE, &model[0][0]);

		// 	glVertexArrayVertexBuffer(VAO, 0, chunk->gpuMesh.VBO, 0, FLOATS_PER_VERTEX * sizeof(GLfloat));
		// 	glVertexArrayElementBuffer(VAO, chunk->gpuMesh.EBO);

		// 	glDrawElements(GL_TRIANGLES, chunk->gpuMesh.numIndices, GL_UNSIGNED_INT, 0);
		// }

		glBindVertexArray(0);
	}
};