#pragma once

#include "Chunk.h"

#include <vector>
#include <unordered_map>

#include "AABB.h"

#include "OpenGL/ShaderProgram.h"

class World {
	static inline int64_t chunkPosToID(const int32_t x, const int32_t z) {
		const int64_t xLong = (int64_t)x & 0xFFFFFFFF;
		return (xLong << 32) | ((int64_t)z & 0xFFFFFFFF);
	}

	std::unordered_map<int64_t, Chunk*> chunks;

public:
	World();
	~World();

	World(const World&) = delete;
	World &operator=(const World&) = delete;

	Chunk *getChunk(const int32_t chunkPosX, const int32_t chunkPosZ);
	std::vector<AABB> getPossibleCollisions(const AABB &bounds);
	void draw(const ShaderProgram &shader);
};