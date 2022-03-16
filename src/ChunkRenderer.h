#pragma once

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "OpenGL/opengl_include.h"
#include "OpenGL/ShaderProgram.h"

#include <unordered_map>
#include <unordered_set>

#include "Coordinates.h"
#include "ChunkBatch.h"

#include <atomic>


class World;

class ChunkRenderer {
private:
	GLuint VAO;

private:
	const uint32_t numLevels; // 1 means storing individual chunks

	std::unordered_map<ChunkPos, ChunkBatch*> chunks; // chunkPos is position of Batch (depends on numLevels, if numLevels==1, pos is identical to actual chunkPos)
	std::unordered_set<ChunkPos> dirtyChunks; // may contain coordinates of non-existing chunks

public:
	std::atomic_bool inUse;

public:
	ChunkRenderer(const uint32_t numLevels);
	~ChunkRenderer();

	ChunkRenderer(const ChunkRenderer&) = delete;
	ChunkRenderer &operator=(const ChunkRenderer&) = delete;

	inline ChunkPos topLevelBatchPos(const ChunkPos &chunkPos) const {
		return ChunkBatch::toBatchPos(chunkPos, numLevels-1);
	}

	inline ChunkBatch *getContainingBatch(const ChunkPos &chunkPos) {
		std::unordered_map<ChunkPos, ChunkBatch*>::iterator it = chunks.find(topLevelBatchPos(chunkPos));
		return (it == chunks.end()) ? nullptr : it->second;
	}
	inline const ChunkBatch *getContainingBatch(const ChunkPos &chunkPos) const {
		std::unordered_map<ChunkPos, ChunkBatch*>::const_iterator it = chunks.find(topLevelBatchPos(chunkPos));
		return (it == chunks.end()) ? nullptr : it->second;
	}

	inline void addChunk(const ChunkPos &chunkPos) {
		ChunkBatch *containingBatch = getContainingBatch(chunkPos);
		if(containingBatch == nullptr) {
			containingBatch = new ChunkBatch(chunkPos, numLevels);
			chunks.insert({topLevelBatchPos(chunkPos), containingBatch});
		}

		if(numLevels > 1) // 1 level means the top-level Batch IS the chunk.
			containingBatch->addChunk(chunkPos);

		markChunkDirty(chunkPos);
		markNeighborsDirty(chunkPos);
	}

	inline void removeChunk(const ChunkPos &chunkPos) {
		ChunkBatch *containingBatch = getContainingBatch(chunkPos);

		if(numLevels > 1) // 1 level means the batch IS the chunk to be removed...
			containingBatch->removeChunk(chunkPos);

		if(numLevels == 1 || containingBatch->numSubMeshes() == 0) {
			chunks.erase(topLevelBatchPos(chunkPos));
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
			ChunkBatch *const batch = getContainingBatch(chunkPos);

			if(batch == nullptr) continue; // Skip non-existing Chunks

			batch->generate(world, chunkPos);
		}
		dirtyChunks.clear();
	}

	inline void regenerateBatches() {
		if(numLevels == 1) return; // Renderer stores individual chunks; there are no Batched Meshes to be generated

		for(const auto &[batchPos, chunkBatch] : chunks)
			if(chunkBatch->dirty)
				chunkBatch->regenerateCombinedMeshes();
	}

	inline void updateGPUBuffers() {
		for(const auto &[batchPos, chunkBatch] : chunks)
			if(chunkBatch->gpuDirty)
				chunkBatch->updateGPUBuffers();
	}

	inline void draw(const ChunkPos &virtualOrigin, const ShaderProgram &shader) {
		glBindVertexArray(VAO);

		for(const auto &[batchPos, chunkBatch] : chunks)
			chunkBatch->draw(virtualOrigin, shader, VAO);

		glBindVertexArray(0);
	}
};