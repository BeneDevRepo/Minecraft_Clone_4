#include "World.h"

#include <iostream> // Debug

#include <tuple>
// #include <algorithm>

#include "glm/ext/matrix_transform.hpp"

constexpr int renderDistance = 16;
World::World():
		chunks() {
	for(int x = -renderDistance; x <= renderDistance; x++) {
		for(int z = -renderDistance; z <= renderDistance; z++) {
			chunks.insert({ChunkPos{x, 0, z}, new Chunk{ x, z } });
		}
	}

	// for(auto it = chunks.begin(); it != chunks.end(); it++)
	// 	it->second->generateMesh(
	// 		getChunk({it->second->chunkPosX-1, 0, it->second->chunkPosZ}),
	// 		getChunk({it->second->chunkPosX+1, 0, it->second->chunkPosZ}),
	// 		getChunk({it->second->chunkPosX,   0, it->second->chunkPosZ-1}),
	// 		getChunk({it->second->chunkPosX,   0, it->second->chunkPosZ+1})
	// 	);
}

World::~World() {
}


// Chunk *World::getChunk(const int32_t chunkPosX, const int32_t chunkPosZ) {
// 	std::unordered_map<int64_t, Chunk*>::iterator it = chunks.find(chunkPosToID(chunkPosX, chunkPosZ));
// 	return (it == chunks.end()) ? nullptr : it->second;
// }

// const Chunk *World::getChunk(const int32_t chunkPosX, const int32_t chunkPosZ) const {
// 	std::unordered_map<int64_t, Chunk*>::const_iterator it = chunks.find(chunkPosToID(chunkPosX, chunkPosZ));
// 	return (it == chunks.end()) ? nullptr : it->second;
// }
Chunk *World::getChunk(const ChunkPos &chunkPos) {
	std::unordered_map<ChunkPos, Chunk*>::iterator it = chunks.find(chunkPos);
	return (it == chunks.end()) ? nullptr : it->second;
}

const Chunk *World::getChunk(const ChunkPos &chunkPos) const {
	std::unordered_map<ChunkPos, Chunk*>::const_iterator it = chunks.find(chunkPos);
	return (it == chunks.end()) ? nullptr : it->second;
}

void World::loadChunksAround(const ChunkPos &center) {
	for(int x = -renderDistance; x <= renderDistance; x++) {
		for(int z = -renderDistance; z <= renderDistance; z++) {
			ChunkPos current{ center.x + x, 0, center.z + z };
			if(getChunk(current) == nullptr)
				chunks.insert({current, new Chunk{ current.x, current.z } });
		}
	}
	for (auto i = chunks.begin(), last = chunks.end(); i != last; ) {
		if (i->second->chunkPosX < center.x - renderDistance
			|| i->second->chunkPosX > center.x + renderDistance
			|| i->second->chunkPosZ < center.z - renderDistance
			|| i->second->chunkPosZ > center.z + renderDistance) {
			delete i->second;
			i = chunks.erase(i);
		} else {
			++i;
		}
	}
	// std::erase_if(chunks, [](const auto& item)->bool {
	// 			auto const& [key, value] = item;
	// 			return value.x;
	// 		}
	// 	);
}


std::vector<AABB> World::getPossibleCollisions(const AABB &bounds) const {
	std::vector<AABB> out;
	for(int y = std::floor(bounds.min.y); y <= std::floor(bounds.max.y); y++) {
		for(int x = std::floor(bounds.min.x); x <= std::floor(bounds.max.x); x++) {
			for(int z = std::floor(bounds.min.z); z <= std::floor(bounds.max.z); z++) {
				const int chunkX = std::floor(x / 16.f);
				const int chunkZ = std::floor(z / 16.f);
				// const Chunk *chunk = getChunk(chunkX, chunkZ);
				const Chunk *chunk = getChunk({chunkX, 0, chunkZ});

				if(chunk == nullptr)
					continue;

				const int relBlockX = x - chunkX * 16;
				const int relBlockZ = z - chunkZ * 16;

				if(chunk->getBlock(relBlockX, y, relBlockZ).type == BlockType::AIR)
					continue;

				const glm::vec3 blockPos(x, y, z);
				const AABB blockAABB = {blockPos, blockPos + glm::vec3(1.f, 1.f, 1.f)};

				if(bounds.collidesWith(blockAABB))
					out.push_back(blockAABB);
			}
		}
	}
	return out;
}

bool World::collidesWith(const AABB &entity, const glm::vec3& displacement, glm::ivec3 &contact_normal, float &contact_time) const {
	const AABB movementBounds = AABB::fromCenter(entity.center() + displacement / 2.f, entity.dimensions() + glm::abs(displacement));
	const std::vector<AABB> initialColliders = getPossibleCollisions(movementBounds);

	contact_time = 2.f;

	for(const AABB &collider_cur : initialColliders) {
		float t_cur;
		glm::ivec3 contact_normal_cur;
		if(DynamicRectVsRect(displacement, entity, collider_cur, contact_normal_cur, t_cur)) {
			if(t_cur >= 0.f && t_cur < contact_time) {
				contact_time = t_cur;
				contact_normal = contact_normal_cur;
			}
		}
	}

	return contact_time < 1.f;
}



void World::draw(const ShaderProgram &shader) {
	for(auto it = chunks.begin(); it != chunks.end(); it++)
		if(it->second->isDirty())
			it->second->generateMesh(
				// getChunk(it->second->chunkPosX-1, it->second->chunkPosZ),
				// getChunk(it->second->chunkPosX+1, it->second->chunkPosZ),
				// getChunk(it->second->chunkPosX,   it->second->chunkPosZ-1),
				// getChunk(it->second->chunkPosX,   it->second->chunkPosZ+1)
				getChunk({it->second->chunkPosX-1, 0, it->second->chunkPosZ}),
				getChunk({it->second->chunkPosX+1, 0, it->second->chunkPosZ}),
				getChunk({it->second->chunkPosX,   0, it->second->chunkPosZ-1}),
				getChunk({it->second->chunkPosX,   0, it->second->chunkPosZ+1})
		);
	// for(int x = -renderDistance; x <= renderDistance; x++) {
	// 	for(int z = -renderDistance; z <= renderDistance; z++) {
	// 		chunks.at(chunkPosToID(x, z))->mesh.bind();
	// 		chunks.at(chunkPosToID(x, z))->mesh.draw();
	// 	}
	// }
	for(auto it = chunks.begin(); it != chunks.end(); it++) {
		const Chunk& chunk = *(it->second);
		const glm::vec3 chunkOffset = glm::vec3(chunk.chunkPosX*16.f, 0.f, chunk.chunkPosZ*16.f);// * 1.02f;
		const glm::mat4 model = glm::translate(glm::mat4(1.f), chunkOffset);
		glProgramUniformMatrix4fv(shader, shader.getUniformLocation("model"), 1, GL_FALSE, &model[0][0]);
		chunk.mesh.bind();
		chunk.mesh.draw();
	}
}