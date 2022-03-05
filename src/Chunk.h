#pragma once

#include "Coordinates.h"

#include "OpenGL/opengl_include.h"

#include <cstdint>
#include <vector>

#include "ChunkRenderer.h"

// #include "World.h"

class World;

enum class BlockType : uint8_t { AIR = 0, GRASS };

struct Block { BlockType type; };

class Chunk {
	friend class World; // world can access chunk
	friend class ChunkMesh; // ChunkMesh can access chunk

public:
	using BlockArray = Block[16][16][16];

private:
	// Block blocks[16][16][16]; // x, y, z
	BlockArray blocks; // x, y, z

public:
	const ChunkPos chunkPos;


public:
	Chunk(const ChunkPos &chunkPos);
	~Chunk();

	Chunk(const Chunk&) = delete;
	Chunk &operator=(const Chunk&) = delete;

	Chunk(Chunk&&) = delete;
	Chunk &operator=(Chunk&&) = delete;

	inline Block getBlock(const glm::ivec3 &pos) const {
		return blocks[pos.x][pos.y][pos.z];
	}

	inline const BlockArray &getBlocks() const {
		return blocks;
	}

	void setBlock(World &world, const glm::ivec3 &blockPos, const Block block);
};