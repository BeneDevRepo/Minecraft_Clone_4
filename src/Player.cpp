#include "Player.h"

#include <vector>

#include "AABB.h"

#include "OpenGL/DebugRenderer.h"
#include <iostream>



// Player::Player(const float x, const float y, const float z, const float FOV):
// 		pos(x, y, z), zoom(FOV),
// 		vel(0.f), inputDir(0.f),
// 		perspective(Perspective::FIRST_PERSON), flying(true),
// 		Yaw(0), Pitch(0),
// 		bodyMesh(StaticMesh::cube(glm::vec3(4.f / 16.f, 1.65f, 8.f / 16.f))) {
// }

Player::Player(const glm::vec3 &absPos, const float FOV):
		// chunkPos{
		// 		(int64_t)std::floor(absPos.x / 16.f),
		// 		(int64_t)std::floor(absPos.y / 16.f),
		// 		(int64_t)std::floor(absPos.z / 16.f)
		// 	},
		// pos(absPos - glm::vec3(chunkPos.x() * 16.f, chunkPos.y() * 16.f, chunkPos.z() * 16.f)),

		pos(absPos),
		// virtualOrigin(0, 0, 0),

		zoom(FOV),
		vel(0.f), inputDir(0.f),
		perspective(Perspective::FIRST_PERSON), flying(true),
		Yaw(0), Pitch(0),
		bodyMesh(StaticMesh::cube(glm::vec3(4.f / 16.f, 1.65f, 8.f / 16.f))) {
}

Player::~Player() {
}

void Player::handleMouseInput(const float dt, const int mouseX, const int mouseY) {
	static int pmouseX = mouseX, pmouseY = mouseY;

	const float MouseSensitivity = .1f;

	Yaw   += (mouseX-pmouseX) * MouseSensitivity;
	Pitch += -(mouseY-pmouseY) * MouseSensitivity;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (Pitch > 89.0f)
		Pitch = 89.0f;
	if (Pitch < -89.0f)
		Pitch = -89.0f;

	pmouseX = mouseX;
	pmouseY = mouseY;


	// Perspective:
	static bool perspChangePressed = false;
	if (GetAsyncKeyState(VK_F5) & 0x8000) {
		if(!perspChangePressed)
			perspective = perspective==Perspective::FIRST_PERSON ? Perspective::THIRD_PERSON : Perspective::FIRST_PERSON;
		perspChangePressed = true;
	} else
		perspChangePressed = false;


	// Flying:
	static bool flyKeyPressedPressed = false;
	if (GetAsyncKeyState('F') & 0x8000) {
		flying ^= !flyKeyPressedPressed;
		flyKeyPressedPressed = true;
	} else
		flyKeyPressedPressed = false;
}




// bool raycast(World &world, const glm::vec3 origin, const glm::vec3 direction, float radius, glm::vec3 &result);




void Player::update(const float dt, World& world) {
	static bool onGround = false;

	{
		inputDir *= 0.f;
		const float yawRad = glm::radians(Yaw);
		const glm::vec2 Forward(cos(yawRad), sin(yawRad));
		const glm::vec2 Right(cos(yawRad + glm::half_pi<float>()), sin(yawRad + glm::half_pi<float>()));
		if (GetAsyncKeyState('W') & 0x8000)
				inputDir += Forward;
		if (GetAsyncKeyState('S') & 0x8000)
				inputDir -= Forward;
		if (GetAsyncKeyState('A') & 0x8000)
				inputDir -= Right;
		if (GetAsyncKeyState('D') & 0x8000)
				inputDir += Right;
	}

	if(onGround && GetAsyncKeyState(' ') & 0x8000)
		vel.y = 10.f;

	// Flying:
	if(flying) {
		if(GetAsyncKeyState(' ') & 0x8000)
			vel.y = 10.f;
		else if(GetAsyncKeyState(VK_SHIFT) & 0x8000)
			vel.y = 10.f;
		else
			vel.y = 0.f;
	}

	glm::vec2 acc2D(0.f);

	// Input:
	if(glm::length(inputDir) > .001f)
		acc2D += glm::normalize(inputDir) * ((GetAsyncKeyState(VK_CONTROL)&0x8000) ? 100.f : 50.f);

	vel += glm::vec3(acc2D.x, 0.f, acc2D.y) * dt;

	// Friction:
	if(glm::length(glm::vec2(vel.x, vel.z)) > .05f) {
		vel.x *= .975;
		vel.z *= .975;
	} else
		vel.x = vel.z = 0.f;

	// Gravity:
	if(!flying)
		vel += glm::vec3(0.f, -20.f + .01f, 0.f) * dt;


	// #####################################################################    DEBUGGING   <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	// static int fr = 0; fr++;
	// if(fr % 120 == 0)
	// 	pos.printDebug();


	// <collision resolution>
	onGround = false;

	for(int i = 0; i < 3; i++) {
		float contact_time;
		glm::ivec3 contact_normal;

		if(!world.collidesWith(getVirtualOrigin(), getAABB(), vel * dt, contact_normal, contact_time))
			break;

		contact_time = std::max(contact_time-.02f, 0.f);

		for(int j = 0; j < 3; j++) {
			if(contact_normal[j]) {
				// pos[j] += vel[j] * dt * contact_time;
				glm::vec3 offset(0.f); offset[j] = vel[j] * dt * contact_time;
				pos += offset;

				vel[j] = 0;
			}
		}
		onGround |= contact_normal.y > 0;

		// DEBUG_RENRERER->box(collider.min, collider.max); // ####################   DEBUG   #####################
	}

	pos += vel * dt; // Update the player position on all Axis that haven't collided with anything
	// </collision resolution>







	// <Test Bullets>
	struct Bullet {
		EntityPos pos;
		// glm::vec3 pos;
		glm::vec3 vel;
		int lifetime;
		inline Bullet(const EntityPos &pos, const glm::vec3 &vel):
			pos(pos), vel(vel), lifetime(240 * 10) {
		}
		inline AABB getAABB(const ChunkPos &virtualOrigin) const {
			// return AABB::fromCenter(pos, glm::vec3(.5f));
			return AABB::fromCenter((pos-virtualOrigin).computeAbsolute(), glm::vec3(.5f));
		}
	};
	static std::vector<Bullet> bullets;


	static int lastShot = 0; lastShot--;
	if(GetAsyncKeyState(VK_MBUTTON) & 0x8000) {
		if(lastShot <= 0) {
			bullets.push_back({EntityPos::compute(getVirtualOrigin(), getViewPos()), getViewDir() * 50.f});
			lastShot = 240 / 10; // 10 Shots Per Second
		}
	} else
		lastShot = 0;


	constexpr float EXPLOSION_RADIUS = 20.f;

	for(std::vector<Bullet>::iterator it = bullets.begin(); it != bullets.end(); ) {
		Bullet &bullet = *it;

		if(bullet.lifetime-- == 0) {
			bullets.erase(it);
			continue;
		}

		float contact_time;
		glm::ivec3 contact_normal;

		if(world.collidesWith(getVirtualOrigin(), bullet.getAABB(getVirtualOrigin()), bullet.vel * dt, contact_normal, contact_time)) {

			const AABB expAABB = AABB::fromCenter((bullet.pos - getVirtualOrigin()).computeAbsolute(), glm::vec3(EXPLOSION_RADIUS * 2));

			const std::vector<AABB> blocks = world.getPossibleCollisions(getVirtualOrigin(), expAABB);
			for(const AABB &col : blocks) {
				const glm::vec3 blockPos = col.min;
				const BlockPos blockPosAbs = BlockPos::compute(getVirtualOrigin(), glm::floor(col.min));

				// const float blockDist = glm::length(blockPos - (bullet.pos - getVirtualOrigin()).computeAbsolute());
				const float blockDist = glm::length(blockPos - (bullet.pos - getVirtualOrigin()).computeAbsolute());
				if(blockDist > EXPLOSION_RADIUS) continue;
				if(blockDist > EXPLOSION_RADIUS-2 && (rand() % 5 == 0)) continue;



				Chunk *const chunk = world.getChunk(blockPosAbs.chunkPos());
				if(chunk == nullptr) continue;

				chunk->setBlock(world, blockPosAbs.blockPos().x, blockPosAbs.blockPos().y, blockPosAbs.blockPos().z, { BlockType::AIR });
			}

			bullets.erase(it);
			continue;
		}

		bullet.pos += bullet.vel * dt; // Update bullet position

		DEBUG_RENRERER->box(bullet.getAABB(getVirtualOrigin()).min, bullet.getAABB(getVirtualOrigin()).max); // ####################   DEBUG   #####################
		++it;
	}
	// </Test Bullets>









	// Object Picking:
	{
		const glm::vec3 viewPos = getViewPos();
		const glm::vec3 viewDir = getViewDir();

		glm::ivec3 currentBlock = glm::floor(viewPos);
		const glm::vec3 deltaT = glm::abs(1.f / viewDir);
		bool hit = false;
		int side = 0;

		glm::ivec3 step {0, 0, 0};
		glm::vec3 sideDist;

		//calculate step and initial sideDist
		if (viewDir.x < 0) {
			step.x = -1;
			sideDist.x = (viewPos.x - currentBlock.x) * deltaT.x;
		} else {
			step.x = 1;
			sideDist.x = (currentBlock.x + 1.0 - viewPos.x) * deltaT.x;
		}

		if (viewDir.y < 0) {
			step.y = -1;
			sideDist.y = (viewPos.y - currentBlock.y) * deltaT.y;
		} else {
			step.y = 1;
			sideDist.y = (currentBlock.y + 1.0 - viewPos.y) * deltaT.y;
		}

		if (viewDir.z < 0) {
			step.z = -1;
			sideDist.z = (viewPos.z - currentBlock.z) * deltaT.z;
		} else {
			step.z = 1;
			sideDist.z = (currentBlock.z + 1.0 - viewPos.z) * deltaT.z;
		}

		for(int i = 0; i < 200 && !hit; i++) {
			// float t = min(sideDistX, sideDistY);

			//jump to next cube
			if (sideDist.x < sideDist.y && sideDist.x < sideDist.z) {
				sideDist.x += deltaT.x;
				currentBlock.x += step.x;
				side = 0;
			} else if (sideDist.y < sideDist.z) {
				sideDist.y += deltaT.y;
				currentBlock.y += step.y;
				side = 1;
			} else {
				sideDist.z += deltaT.z;
				currentBlock.z += step.z;
				side = 2;
			}

			//Check if ray has hit a wall
			const BlockPos blockPosAbsolute = BlockPos::compute(getVirtualOrigin(), currentBlock);
			// const ChunkPos chunkPosRel = ChunkPos{
			// 	(int64_t)std::floor(currentBlock.x / 16.f),
			// 	(int64_t)std::floor(currentBlock.y / 16.f),
			// 	(int64_t)std::floor(currentBlock.z / 16.f)};

			// const ChunkPos chunkPos = ChunkPos{
			// 	(int64_t)std::floor(currentBlock.x / 16.f),
			// 	(int64_t)std::floor(currentBlock.y / 16.f),
			// 	(int64_t)std::floor(currentBlock.z / 16.f)}
			// 	+ getVirtualOrigin();
			// const ChunkPos chunkPos = chunkPosRel + getVirtualOrigin();

			// Chunk *const chunk = world.getChunk(chunkPos);
			Chunk *const chunk = world.getChunk(blockPosAbsolute.chunkPos());
			if(chunk == nullptr)
				continue;

			// const glm::ivec3 blockPosRel(
			// 	currentBlock.x - chunkPosRel.x() * 16,
			// 	currentBlock.y - chunkPosRel.y() * 16,
			// 	currentBlock.z - chunkPosRel.z() * 16);

			// if(chunk->getBlock(blockPosRel.x, blockPosRel.y, blockPosRel.z).type != BlockType::AIR) {
			if(chunk->getBlock(blockPosAbsolute.blockPos()).type != BlockType::AIR) {
				hit = true;
			}
		}
		if(hit) {
			DEBUG_RENRERER->box(
					glm::vec3(currentBlock.x, currentBlock.y, currentBlock.z),
					glm::vec3(currentBlock.x+1, currentBlock.y+1, currentBlock.z+1)
				);

			if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
				const BlockPos blockPosAbsolute = BlockPos::compute(getVirtualOrigin(), currentBlock);
				// const ChunkPos chunkPos{
				// 	(int64_t)std::floor(currentBlock.x / 16.f),
				// 	(int64_t)std::floor(currentBlock.y / 16.f),
				// 	(int64_t)std::floor(currentBlock.z / 16.f) };
				// Chunk *const chunk = world.getChunk(chunkPos);
				Chunk *const chunk = world.getChunk(blockPosAbsolute.chunkPos());

				if(chunk != nullptr) {
					// const glm::ivec3 blockPosRel(
					// 	currentBlock.x - chunkPos.x() * 16,
					// 	currentBlock.y - chunkPos.y() * 16,
					// 	currentBlock.z - chunkPos.z() * 16);
					const glm::ivec3 blockPosRel = blockPosAbsolute.blockPos();

					chunk->setBlock(world, blockPosRel.x, blockPosRel.y, blockPosRel.z, {BlockType::AIR});
				}
			}

			glm::ivec3 normal(0);
			normal[side] = viewDir[side] > 0 ? -1 : 1;
			const glm::ivec3 targetBlock = currentBlock + normal;

			// DEBUG_RENRERER->box(
			// 		glm::vec3(targetBlock.x, targetBlock.y, targetBlock.z),
			// 		glm::vec3(targetBlock.x+1, targetBlock.y+1, targetBlock.z+1)
			// 	);

			if(GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
				const BlockPos blockPosAbsolute = BlockPos::compute(getVirtualOrigin(), targetBlock);

				AABB blockAABB{targetBlock,
						{targetBlock.x+1, targetBlock.y+1, targetBlock.z+1}};

				if(!blockAABB.collidesWith(getAABB())) {
					// const ChunkPos chunkPos{
					// 	(int64_t)std::floor(targetBlock.x / 16.f),
					// 	(int64_t)std::floor(targetBlock.y / 16.f),
					// 	(int64_t)std::floor(targetBlock.z / 16.f) };

					// Chunk *const chunk = world.getChunk(chunkPos);
					Chunk *const chunk = world.getChunk(blockPosAbsolute.chunkPos());

					if(chunk != nullptr) {
						// const glm::ivec3 blockPosRel(
						// 	targetBlock.x - chunkPos.x() * 16,
						// 	targetBlock.y - chunkPos.y() * 16,
						// 	targetBlock.z - chunkPos.z() * 16);
						const glm::ivec3 blockPosRel = blockPosAbsolute.blockPos();

						chunk->setBlock(world, blockPosRel.x, blockPosRel.y, blockPosRel.z, {BlockType::GRASS});
					}
				}
			}
		}
	}
}

void Player::draw() const {
	bodyMesh.bind();
	bodyMesh.draw();
}