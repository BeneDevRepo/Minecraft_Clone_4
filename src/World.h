#pragma once

#include <vector>
#include <unordered_map>

#include "Chunk.h"
#include "Coordinates.h"

#include "AABB.h"

#include "OpenGL/ShaderProgram.h"

#include "ChunkRenderer.h"

// class Chunk;

class World {
private:
	int renderDistance;
	std::unordered_map<ChunkPos, Chunk*> chunks;
	ChunkRenderer renderer;

public:
	World();
	~World();

	World(const World&) = delete;
	World &operator=(const World&) = delete;

public:
	inline Chunk *getChunk(const ChunkPos &chunkPos) {
		std::unordered_map<ChunkPos, Chunk*>::iterator it = chunks.find(chunkPos);
		return (it == chunks.end()) ? nullptr : it->second;
	}
	inline const Chunk *getChunk(const ChunkPos &chunkPos) const {
		std::unordered_map<ChunkPos, Chunk*>::const_iterator it = chunks.find(chunkPos);
		return (it == chunks.end()) ? nullptr : it->second;
	}

	void loadChunksAround(const ChunkPos &center);


public:
	std::vector<AABB> getPossibleCollisions(const ChunkPos &virtualOrigin, const AABB &bounds) const;
	bool collidesWith(const ChunkPos &virtualOrigin, const AABB &entity, const glm::vec3& displacement, glm::ivec3 &contact_normal, float &contact_time) const;


public:
	inline void markChunkDirty(const ChunkPos &chunkPos) {
		renderer.markChunkDirty(chunkPos);
	}
	inline void markNeighborsDirty(const ChunkPos &chunkPos) {
		renderer.markNeighborsDirty(chunkPos);
	}
	void draw(const ChunkPos &virtualOrigin, const glm::mat4 &pv, const ShaderProgram &shader);
};