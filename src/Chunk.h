#pragma once

#include "OpenGL/StaticMesh.h"

#include <cstdint>

class World;

enum class BlockType : uint8_t { AIR = 0, GRASS };

struct Block {
	BlockType type;
};

class Chunk {
private:
	Block blocks[16][256][16]; // x, y, z
	bool dirty;

public:
	int32_t chunkPosX, chunkPosZ;

public:
	StaticMesh mesh;

public:
	Chunk(const int32_t chunkPosX, const int32_t chunkPosZ);
	~Chunk();

	Chunk(const Chunk&) = delete;
	Chunk &operator=(const Chunk&) = delete;

	Chunk(Chunk&& other);
	Chunk &operator=(Chunk&& other);

	void generateMesh(const Chunk *const xMinus, const Chunk *const xPlus, const Chunk *const zMinus, const Chunk *const zPlus);
	Block getBlock(const uint8_t x, const uint8_t y, const uint8_t z) const;
	void setBlock(World &world, const uint8_t x, const uint8_t y, const uint8_t z, const Block block);

	inline bool isDirty() const {
		return dirty;
	}
};