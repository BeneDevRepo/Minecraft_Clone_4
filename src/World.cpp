#include "World.h"

#include <iostream> // Debug

#include <tuple>

#include "glm/ext/matrix_transform.hpp"

constexpr int renderDistance = 16;
World::World():
		chunks() {
	for(int x = -renderDistance; x <= renderDistance; x++) {
		for(int z = -renderDistance; z <= renderDistance; z++) {
			chunks.insert({chunkPosToID(x, z), new Chunk{ x, z } });
		}
	}

	for(auto it = chunks.begin(); it != chunks.end(); it++)
		it->second->generateMesh(
			getChunk(it->second->chunkPosX-1, it->second->chunkPosZ),
			getChunk(it->second->chunkPosX+1, it->second->chunkPosZ),
			getChunk(it->second->chunkPosX,   it->second->chunkPosZ-1),
			getChunk(it->second->chunkPosX,   it->second->chunkPosZ+1)
		);
}

World::~World() {
}

Chunk *World::getChunk(const int32_t chunkPosX, const int32_t chunkPosZ) {
	std::unordered_map<int64_t, Chunk*>::iterator it = chunks.find(chunkPosToID(chunkPosX, chunkPosZ));
	return (it == chunks.end()) ? nullptr : it->second;
}

std::vector<AABB> World::getPossibleCollisions(const AABB &bounds) {
	std::vector<AABB> out;
	for(int y = std::floor(bounds.min.y); y <= std::floor(bounds.max.y); y++) {
		for(int x = std::floor(bounds.min.x); x <= std::floor(bounds.max.x); x++) {
			for(int z = std::floor(bounds.min.z); z <= std::floor(bounds.max.z); z++) {
				const int chunkX = std::floor(x / 16.f);
				const int chunkZ = std::floor(z / 16.f);
				const Chunk *chunk = getChunk(chunkX, chunkZ);

				if(chunk == nullptr)
					continue;
				
				const int relBlockX = x - chunkX * 16;
				const int relBlockZ = z - chunkZ * 16;
				

				if(chunk->getBlock(relBlockX, y, relBlockZ).type == BlockType::AIR)
					continue;

				const glm::vec3 blockPos(x, y, z);
				AABB blockAABB = {blockPos, blockPos + glm::vec3(1.f, 1.f, 1.f)};

				out.push_back(blockAABB);
			}
		}
	}
	return out;
}

void World::draw(const ShaderProgram &shader) {
	for(auto it = chunks.begin(); it != chunks.end(); it++)
		if(it->second->isDirty())
			it->second->generateMesh(
				getChunk(it->second->chunkPosX-1, it->second->chunkPosZ),
				getChunk(it->second->chunkPosX+1, it->second->chunkPosZ),
				getChunk(it->second->chunkPosX,   it->second->chunkPosZ-1),
				getChunk(it->second->chunkPosX,   it->second->chunkPosZ+1)
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