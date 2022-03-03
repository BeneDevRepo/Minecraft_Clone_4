#include "World.h"

#include "glm/ext/matrix_transform.hpp"

// #include "FrustumCull.h"

#include <algorithm>

#include <iostream> // Debug
#include "Profiling/Profiler.h"


World::World():
		// renderDistance(10),
		renderDistance(5),
		chunks{},
		renderer{} {
}

World::~World() {
}

void World::loadChunksAround(const ChunkPos &center) {
	{
		static ChunkPos lastCenter;
		static bool chunksLoaded;
		if(chunksLoaded && center == lastCenter) return;
		lastCenter = center;
		chunksLoaded = true;
	}

	// if(!(GetAsyncKeyState('R') & 0x8000))
	// 	return;

	// // Load new Chunks:
	for(int x = -renderDistance; x <= renderDistance; x++) {
		for(int y = -renderDistance; y <= renderDistance; y++) {
			for(int z = -renderDistance; z <= renderDistance; z++) {
				const ChunkPos current{ center.x() + x, center.y() + y, center.z() + z };

				if(getChunk(current) == nullptr) {
					chunks.insert({current, new Chunk(current)});
					renderer.addChunk(*this, current);
				}
			}
		}
	}

	// Unload distant chunks:
	for (auto i = chunks.begin(), last = chunks.end(); i != last; ) {
		const ChunkPos &chunkPos = i->second->chunkPos;
		if (	   chunkPos.x() < center.x() - renderDistance
				|| chunkPos.y() < center.y() - renderDistance
				|| chunkPos.z() < center.z() - renderDistance
				|| chunkPos.x() > center.x() + renderDistance
				|| chunkPos.y() > center.y() + renderDistance
				|| chunkPos.z() > center.z() + renderDistance) {
			Chunk *const chunk = i->second;
			i = chunks.erase(i);

			renderer.removeChunk(chunk->chunkPos);
			delete chunk;
		} else {
			++i;
		}
	}
}

// bounds + returned AABBS are relative to virtualOrigin
std::vector<AABB> World::getPossibleCollisions(const ChunkPos &virtualOrigin, const AABB &bounds) const {
	std::vector<AABB> out;
	for(int y = std::floor(bounds.min.y); y < std::ceil(bounds.max.y); y++) {
		for(int x = std::floor(bounds.min.x); x < std::ceil(bounds.max.x); x++) {
			for(int z = std::floor(bounds.min.z); z < std::ceil(bounds.max.z); z++) {
				const glm::ivec3 blockPosRel(x, y, z);
				const BlockPos blockPosAbs = BlockPos::compute(virtualOrigin, blockPosRel);

				const Chunk *chunk = getChunk(blockPosAbs.chunkPos());

				if(chunk == nullptr) continue; // can't collide if not in chunk

				if(chunk->getBlock(blockPosAbs.blockPos()).type == BlockType::AIR) continue; // can't collide with air

				const AABB blockAABB = {blockPosRel, blockPosRel + glm::ivec3(1, 1, 1)};

				if(bounds.collidesWith(blockAABB))
					out.push_back(blockAABB);
			}
		}
	}
	return out;
}

// Entity AABB is relative to virtualOrigin
bool World::collidesWith(const ChunkPos &virtualOrigin, const AABB &entity, const glm::vec3& displacement, glm::ivec3 &contact_normal, float &contact_time) const {
	const AABB movementBounds = AABB::fromCenter(entity.center() + displacement / 2.f, entity.dimensions() + glm::abs(displacement));
	const std::vector<AABB> initialColliders = getPossibleCollisions(virtualOrigin, movementBounds);

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


void World::draw(const ChunkPos &virtualOrigin, const glm::mat4 &pv, const ShaderProgram &shader) {
	Profiler::get().startSegment("Regenerate Meshes");
	renderer.regenerateChunks(*this);

	Profiler::get().startSegment("Render Meshes");
	renderer.draw(virtualOrigin, shader);
}