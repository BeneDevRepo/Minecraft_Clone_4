#pragma once

#include "Chunk.h"

#include <vector>
#include <unordered_map>

#include "AABB.h"

#include "OpenGL/ShaderProgram.h"

#include "Coordinates.h"

class World {
	std::unordered_map<ChunkPos, Chunk*> chunks;
	// ChunkPos virtualOrigin;

public:
	World();
	// World(ChunkPos virtualOrigin);
	~World();

	World(const World&) = delete;
	World &operator=(const World&) = delete;

	Chunk *getChunk(const ChunkPos &chunkPos);
	const Chunk *getChunk(const ChunkPos &chunkPos) const;

	void loadChunksAround(const ChunkPos &center);

	std::vector<AABB> getPossibleCollisions(const ChunkPos &virtualOrigin, const AABB &bounds) const;
	bool collidesWith(const ChunkPos &virtualOrigin, const AABB &entity, const glm::vec3& displacement, glm::ivec3 &contact_normal, float &contact_time) const;

	void draw(const ChunkPos &virtualOrigin, const ShaderProgram &shader);
};