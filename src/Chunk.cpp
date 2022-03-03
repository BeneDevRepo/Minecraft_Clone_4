#include "Chunk.h"

#include "World.h"

#include <cmath>
#include <vector>

#include <iostream> // Debugging




Chunk::Chunk(const ChunkPos &chunkPos):
		chunkPos(chunkPos),
		blocks{} {

	static constexpr auto getHeight = [](const int64_t blockX, const int64_t blockZ)->uint8_t { return 8 + (int)(3*sin((blockX + blockZ * .5) * .05) + sin(blockX * .1)); };

	for(int x = 0; x < 16; x++) {
		for(int z = 0; z < 16; z++) {
			const int64_t absX = chunkPos.x() * 16 + x;
			const int64_t absZ = chunkPos.z() * 16 + z;
			for(int y = 0; y < 16; y++) {
				// blocks[x][y][z] = { (y + chunkPos.y() * 16 < 5) ? BlockType::GRASS : BlockType::AIR };
				// if(chunkPos.y() == 0 && y==5)
				// 	if((x % 8 < 4) == (z % 8 < 4))
				// 		blocks[x][y][z] = { BlockType::GRASS };
				blocks[x][y][z] = { (y + chunkPos.y() * 16 < getHeight(absX, absZ)) ? BlockType::GRASS : BlockType::AIR };
			}
		}
	}
}

Chunk::~Chunk() {
}

void Chunk::setBlock(World &world, const glm::ivec3 &blockPos, const Block block) {
	blocks[blockPos.x][blockPos.y][blockPos.z] = block;
	world.markChunkDirty(chunkPos);
	world.markNeighborsDirty(chunkPos);
}