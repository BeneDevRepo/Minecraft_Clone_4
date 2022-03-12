#pragma once

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "OpenGL/opengl_include.h"
#include "OpenGL/ShaderProgram.h"

#include <cstdint>

#include "Mesh.h"
#include "GPUMesh.h"
#include "Coordinates.h"
#include "ChunkMeshGenerator.h"

class ChunkBatch {
	friend class ChunkRenderer;

private:
	const uint32_t level; // level 0 Batches represent single Chunks
	const ChunkPos batchPos;

	ChunkBatch *subBatches[8]; // [level > 0] Batches have up to 8 sub-Batches, otherwise unused

	Mesh mesh;
	bool dirty; // true if mesh not up-to-date

	GPUMesh gpuMesh;
	bool gpuDirty; // true if GPU Buffers not up-to-date


public:
	static inline ChunkPos toBatchPos(const ChunkPos &chunkPos, const int32_t level) {
		constexpr auto floorDiv = [](int64_t a, int64_t b) -> int64_t {
				const bool remainder = (a % b) != 0;
				const bool resultNegative = (a < 0) != (b < 0);
				return a / b - (remainder && resultNegative);
			};

			const int64_t divisor = 1 << level; // 2 ^ level

			return {
				floorDiv(chunkPos.x(), divisor),
				floorDiv(chunkPos.y(), divisor),
				floorDiv(chunkPos.z(), divisor)
			};
	}
	static inline ChunkPos toChunkPos(const ChunkPos &batchPos, const uint32_t level) {
			return batchPos * (1 << level);
		}

	inline ChunkPos toRelativeSubBatchPos(const ChunkPos &chunkPos) const {
		return toBatchPos(chunkPos - toChunkPos(batchPos, level), level-1);
	}

public:
	inline ChunkBatch(const ChunkPos &chunkPos, const uint32_t numSubchunkLevels):
			level(numSubchunkLevels-1),
			batchPos(toBatchPos(chunkPos, numSubchunkLevels-1)), // can't use level inside initializer list
			dirty(true), gpuDirty(false) {
		if(level > 0)
			for(int i = 0; i < 8; i++)
				subBatches[i] = nullptr;
	}
	inline ~ChunkBatch() {
	}

	ChunkBatch(ChunkBatch&) = delete;
	ChunkBatch &operator=(ChunkBatch&) = delete;

	ChunkBatch(ChunkBatch&&) = delete;
	ChunkBatch &operator=(ChunkBatch&&) = delete;

public:
	inline uint8_t numSubMeshes() const {
		if(level == 0) return 0; // level 0 Batches don't contain sub Batches

		uint8_t num = 0;
		for(int i = 0; i < 8; i++)
			num += subBatches[i] != nullptr;
		return num;
	}

	inline Mesh &getMesh() {
		if(numSubMeshes() == 1) // Batches with exactly 1 SubMesh just reference that Batch's content
			for(int i = 0; i < 8; i++)
				if(subBatches[i])
					return subBatches[i]->getMesh();
		return mesh;
	}
	inline const Mesh &getMesh() const {
		if(numSubMeshes() == 1) // Batches with exactly 1 SubMesh just reference that Batch's content
			for(int i = 0; i < 8; i++)
				if(subBatches[i])
					return subBatches[i]->getMesh();
		return mesh;
	}
	inline GPUMesh &getGPUMesh() {
		if(numSubMeshes() == 1) // Batches with exactly 1 SubMesh just reference that Batch's content
			for(int i = 0; i < 8; i++)
				if(subBatches[i])
					return subBatches[i]->getGPUMesh();
		return gpuMesh;
	}
	inline const GPUMesh &getGPUMesh() const {
		if(numSubMeshes() == 1) // Batches with exactly 1 SubMesh just reference that Batch's content
			for(int i = 0; i < 8; i++)
				if(subBatches[i])
					return subBatches[i]->getGPUMesh();
		return gpuMesh;
	}
	inline const ChunkPos absoluteMeshPosition() const {
		if(numSubMeshes() == 1) // Batches with exactly 1 SubMesh just reference that Batch's content
			for(int i = 0; i < 8; i++)
				if(subBatches[i])
					return subBatches[i]->absoluteMeshPosition();
		return toChunkPos(batchPos, level);
	}
	inline bool isDirty() const {
		if(numSubMeshes() == 1) // Batches with exactly 1 SubMesh just reference that Batch's content
			for(int i = 0; i < 8; i++)
				if(subBatches[i])
					return subBatches[i]->isDirty();
		return dirty;
	}
	inline bool isGPUDirty() const {
		if(numSubMeshes() == 1) // Batches with exactly 1 SubMesh just reference that Batch's content
			for(int i = 0; i < 8; i++)
				if(subBatches[i])
					return subBatches[i]->isGPUDirty();
		return gpuDirty;
	}

public:
	inline void addChunk(const ChunkPos &chunkPos) {
		const ChunkPos chunkPosRel = toRelativeSubBatchPos(chunkPos);
		const uint8_t index = chunkPosRel.z() * 4 + chunkPosRel.y() * 2 + chunkPosRel.x();

		if(subBatches[index] == nullptr)
			subBatches[index] = new ChunkBatch(chunkPos, level);

		if(level-1 > 0) // only add chunk to SubBatch if subBatch doesn't represent a single Chunk (if level of SubBatch > 0)
			subBatches[index]->addChunk(chunkPos);
	}

	inline void removeChunk(const ChunkPos &chunkPos) {
		const ChunkPos chunkPosRel = toRelativeSubBatchPos(chunkPos);
		const uint8_t index = chunkPosRel.z() * 4 + chunkPosRel.y() * 2 + chunkPosRel.x();

		if(level == 1) {
			delete subBatches[index];
			subBatches[index] = nullptr;
		} else {
			subBatches[index]->removeChunk(chunkPos);

			if(subBatches[index]->numSubMeshes() == 0) {
				delete subBatches[index];
				subBatches[index] = nullptr;
			}
		}

		dirty = true;
	}


public:
	inline void generateCombinedMesh() {
		if(numSubMeshes() <= 1) return;

		constexpr GLsizei FLOATS_PER_VERTEX = 11;

		int numCombinedVertices = 0;
		int numCombinedIndices = 0;
		for(int i = 0; i < 8; i++) {
			if(subBatches[i] == nullptr) continue; // Skip non-existing Batches

			numCombinedVertices += subBatches[i]->getMesh().numVertices;
			numCombinedIndices += subBatches[i]->getMesh().numIndices;
		}

		mesh.resize(FLOATS_PER_VERTEX, numCombinedVertices, numCombinedIndices); // allocate buffers + set vertex/index counts

		uint32_t numVerticesCopied = 0;
		uint32_t numIndicesCopied = 0;
		for(int chunkIndex = 0; chunkIndex < 8; chunkIndex++) {
			if(subBatches[chunkIndex] == nullptr) continue; // Skip non-existing Batches

			const ChunkBatch &batch = *subBatches[chunkIndex];
			const Mesh &subMesh = batch.getMesh();

			for(int i = 0; i < subMesh.numIndices; i++, numIndicesCopied++)
				mesh.indices[numIndicesCopied] = subMesh.indices[i] + numVerticesCopied; // Copy (adjusted) indices

			for(int i = 0; i < subMesh.numVertices; i++, numVerticesCopied++) {
				memcpy( mesh.vertices + (FLOATS_PER_VERTEX * numVerticesCopied), // Copy vertices
						subMesh.vertices + (FLOATS_PER_VERTEX * i),
						FLOATS_PER_VERTEX * sizeof(GLfloat));

				const ChunkPos offset = batch.absoluteMeshPosition() - absoluteMeshPosition();
				for(int dimension = 0; dimension < 3; dimension++)
					mesh.vertices[numVerticesCopied * FLOATS_PER_VERTEX + dimension] += offset[dimension] * 16.f;
			}
		}

		dirty = false;
		gpuDirty = true;
	}

	inline bool generate(const World &world, const ChunkPos &chunkPos) { // returns true if any mesh was changed     
		if(level == 0) {
			generateChunkMesh(world, chunkPos, mesh);
			gpuDirty = true;
			return true;
		}

		const ChunkPos chunkPosRel = toRelativeSubBatchPos(chunkPos);
		const uint8_t index = chunkPosRel.z() * 4 + chunkPosRel.y() * 2 + chunkPosRel.x();

		if(subBatches[index] != nullptr) {
			if(subBatches[index]->generate(world, chunkPos));
			{
				dirty = true;
				return true;
			}
		}

		return false;
	}

	inline void regenerateCombinedMeshes() {
		if(level > 1) // Subbatches of level 1 Batches are individual chunks and cannot generate combined meshes
			for(int i = 0; i < 8; i++)
				if(subBatches[i] != nullptr)
					if(subBatches[i]->dirty)
						subBatches[i]->regenerateCombinedMeshes();

		generateCombinedMesh();
	}

	inline void updateGPUBuffers() {
		if(level > 0)
			for(int i = 0; i < 8; i++)
				if(subBatches[i] != nullptr)
					// if(subBatches[i]->gpuDirty)
					subBatches[i]->updateGPUBuffers();

		if(level == 0 || numSubMeshes() > 1) {
			if(gpuDirty) {
				gpuMesh.upload(mesh);
				gpuDirty = false;
			}
		}
	}

public:
	inline void draw(const ChunkPos &virtualOrigin, const ShaderProgram &shader, const GLuint VAO) {
		// constexpr GLsizei FLOATS_PER_VERTEX = 11;

		if(level == 0 || (!isDirty() && !isGPUDirty())) { // If possible, draw only one Mesh:
		// if(level == 0 || (false)) { // If possible, draw only one Mesh:
			const glm::mat4 model = glm::translate(glm::mat4(1.f), (absoluteMeshPosition() - virtualOrigin).toVec3() * 16.f);

			glProgramUniformMatrix4fv(shader, shader.getUniformLocation("model"), 1, GL_FALSE, &model[0][0]);

			getGPUMesh().draw(VAO);
		} else { // Otherwise delegate:
			for(int i = 0; i < 8; i++)
				if(subBatches[i] != nullptr)
					subBatches[i]->draw(virtualOrigin, shader, VAO);
		}
	}
};