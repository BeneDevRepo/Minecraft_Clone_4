#include "World.h"

#include "glm/ext/matrix_transform.hpp"

// #include "FrustumCull.h"

#include <algorithm>


#include "Threading/mingw.thread.h"


#include <iostream> // Debug
#include "Profiling/Profiler.h"


#include <vector>

struct InterThreadMessage {
	enum class Command {
		AddChunkToRenderer,
		RemoveChunkFromRenderer,
		MarkChunkDirty,
	};

	Command command;
	ChunkPos targetChunk;

	inline InterThreadMessage(const Command _command, const ChunkPos _targetChunk): command(_command), targetChunk(_targetChunk) { }
	inline InterThreadMessage(const InterThreadMessage &other): InterThreadMessage(command, targetChunk) { }
};

std::vector<InterThreadMessage> toRenderer, fromRenderer;

World::World():
		// renderDistance(20),
		// renderDistance(16),
		renderDistance(5),
		// renderDistance(3),
		// chunks{},
		renderer(4) {
		// renderer(1) {

	numUploadFailures.store(0);

	std::thread *generator = new std::thread([](ChunkRenderer &renderer, World &world, std::atomic_uint32_t &numCommFailes) {
		for(;;) {
			for(; numCommFailes.load(std::memory_order::memory_order_relaxed) >= 5; );
			for(bool expected = false; !renderer.inUse.compare_exchange_weak(expected, true); expected = false);


			// for(const InterThreadMessage &message : toRenderer) {
			// 	switch(message.command) {
			// 		case InterThreadMessage::Command::AddChunkToRenderer:
			// 			renderer.addChunk(message.targetChunk);
			// 			break;

			// 		// case InterThreadMessage::Command::RemoveChunkFromRenderer:
			// 		// 	renderer.removeChunk(message.targetChunk);
			// 		// 	break;

			// 		case InterThreadMessage::Command::MarkChunkDirty:
			// 			renderer.markChunkDirty(message.targetChunk);
			// 			break;
			// 	}
			// }
			// toRenderer.clear();


			// Profiler::get().startSegment("Regenerate Chunk Meshes");
			renderer.regenerateChunks(world);

			// Profiler::get().startSegment("Regenerate Batch Meshes");
			renderer.regenerateBatches();

			renderer.inUse.store(false);
			// Sleep(100);
			// printf("Generating\n");
		}
	}, std::ref(renderer), std::ref(*this), std::ref(numUploadFailures));
	// generator.join();
}

World::~World() {
}

void World::loadChunksAround(const ChunkPos &center) {
	{
		static ChunkPos lastCenter;
		static bool chunksLoaded = false;
		if(chunksLoaded && center == lastCenter) return;
		lastCenter = center;
		chunksLoaded = true;
	}

	// // Load new Chunks:
	for(int x = -renderDistance; x <= renderDistance; x++) {
		for(int y = -renderDistance; y <= renderDistance; y++) {
			for(int z = -renderDistance; z <= renderDistance; z++) {
				const ChunkPos current{ center.x() + x, center.y() + y, center.z() + z };

				if(getChunk(current) == nullptr) {
					chunks.insert({current, new Chunk(current)});

					// renderer.addChunk(current);
					newChunks.insert(current);
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
			// i = chunks.erase(i); // -----------------------------
			++i;

			// renderer.removeChunk(chunk->chunkPos); // old
			// deletedChunks.insert(chunk->chunkPos);
			// delete chunk; // -----------------------------
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

	// if(bool expected = false; renderer.inUse.compare_exchange_weak(expected, true)) {
	// 	Profiler::get().startSegment("Regenerate Chunk Meshes");
	// 	renderer.regenerateChunks(*this);

	// 	Profiler::get().startSegment("Regenerate Batch Meshes");
	// 	renderer.regenerateBatches();

	// 	renderer.inUse.store(false);
	// }

	Profiler::get().startSegment("Update GPU Buffers");
	if(bool expected = false; renderer.inUse.compare_exchange_weak(expected, true)) {
		renderer.updateGPUBuffers();

		// for(const ChunkPos &chunkPos : deletedChunks) {
		for(auto it = deletedChunks.begin(); it != deletedChunks.end(); ) {
			const ChunkPos &chunkPos = *it;
			if(newChunks.erase(chunkPos) > 0) { // erase returns number of erased Elements
				// deletedChunks.erase(chunkPos);
				it = deletedChunks.erase(it);
				continue;
			}
			++it;
		}


		// for(const ChunkPos &chunkPos : newChunks)
		// 	toRenderer.push_back(InterThreadMessage{InterThreadMessage::Command::AddChunkToRenderer, chunkPos});
		// // for(const ChunkPos &chunkPos : deletedChunks)
		// // 	toRenderer.push_back({InterThreadMessage::Command::RemoveChunkFromRenderer, chunkPos});
		// for(const ChunkPos &chunkPos : dirtyChunks)
		// 	toRenderer.push_back(InterThreadMessage{InterThreadMessage::Command::MarkChunkDirty, chunkPos});

		for(const ChunkPos &chunkPos : newChunks)
			renderer.addChunk(chunkPos);
		// for(const ChunkPos &chunkPos : deletedChunks)
		// 	renderer.removeChunk(chunkPos);
		for(const ChunkPos &chunkPos : dirtyChunks)
			renderer.markChunkDirty(chunkPos);

		newChunks.clear();
		deletedChunks.clear();
		dirtyChunks.clear();


		renderer.inUse.store(false);

		numUploadFailures.store(0);
	} else
		numUploadFailures.fetch_add(1);

	Profiler::get().startSegment("Render Meshes");
	renderer.draw(virtualOrigin, shader);
}