#include "Player.h"

#include <vector>
#include <algorithm>

#include "AABB.h"

#include "OpenGL/DebugRenderer.h"


Player::Player(const float x, const float y, const float z, const float FOV):
		pos(x, y, z), zoom(FOV),
		vel(0.f), inputDir(0.f),
		perspective(Perspective::FIRST_PERSON), flying(true),
		Yaw(0), Pitch(0),
		bodyMesh(StaticMesh::cube(false, glm::vec3(4.f / 16.f, 1.65f, 8.f / 16.f))) {
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


bool RayVsRect(const glm::vec3 &ray_origin, const glm::vec3 &ray_dir, const AABB &target, glm::vec3 &contact_point, glm::vec3 &contact_normal, float &t_hit_near) {
	contact_normal = { 0.f, 0.f, 0.f };
	contact_point = { 0.f, 0.f, 0.f };

	// Cache division
	glm::vec3 invdir = 1.f / ray_dir;

	// Calculate intersections with rectangle bounding axes
	glm::vec3 t_near = (target.min - ray_origin) * invdir;
	glm::vec3 t_far = (target.max - ray_origin) * invdir;

	if (std::isnan(t_far.y) || std::isnan(t_far.x) || std::isnan(t_far.z)) return false;
	if (std::isnan(t_near.y) || std::isnan(t_near.x) || std::isnan(t_near.z)) return false;

	// Sort distances
	if (t_near.x > t_far.x) std::swap(t_near.x, t_far.x);
	if (t_near.y > t_far.y) std::swap(t_near.y, t_far.y);
	if (t_near.z > t_far.z) std::swap(t_near.z, t_far.z);

	// Early rejection
	if (t_near.x > t_far.y || t_near.y > t_far.x) return false; // ORIG
	if (t_near.x > t_far.z || t_near.z > t_far.x) return false;
	if (t_near.y > t_far.z || t_near.z > t_far.y) return false;

	// Closest 'time' will be the first contact
	t_hit_near = std::max(std::max(t_near.x, t_near.y), t_near.z);

	// Furthest 'time' is contact on opposite side of target
	const float t_hit_far = std::min(std::min(t_far.x, t_far.y), t_far.z);

	// Reject if ray direction is pointing away from object
	if (t_hit_far < 0)
		return false;

	// Contact point of collision from parametric line equation
	contact_point = ray_origin + t_hit_near * ray_dir;

	// --- Dimensional extrapolation:
	if (t_near.x > t_near.y && t_near.x > t_near.z) {
		if (invdir.x < 0)
			contact_normal = { 1, 0, 0 };
		else
			contact_normal = { -1, 0, 0 };
	} else if (t_near.y > t_near.x && t_near.y > t_near.z) {
		if (invdir.y < 0)
			contact_normal = { 0, 1, 0 };
		else
			contact_normal = { 0, -1, 0 };
	} else if (t_near.z > t_near.x && t_near.z > t_near.y) {
		if (invdir.z < 0)
			contact_normal = { 0, 0, 1 };
		else
			contact_normal = { 0, 0, -1 };
	}

	// Note if t_near == t_far, collision is principly in a diagonal
	// so pointless to resolve. By returning a CN={0,0} even though its
	// considered a hit, the resolver wont change anything.

	return true;
}

bool DynamicRectVsRect(const glm::vec3 displacement, const glm::vec3 dynamicCenter, const glm::vec3 dynamicDimensions, const AABB &r_static, glm::vec3 &contact_point, glm::vec3 &contact_normal, float &contact_time) {
	// Check if dynamic rectangle is actually moving - we assume rectangles are NOT in collision to start
	if (displacement.x == 0 && displacement.y == 0 && displacement.z == 0)
		return false;

	// Expand target rectangle by source dimensions
	AABB expanded_target;
	expanded_target.min = r_static.min - dynamicDimensions / 2.f;
	expanded_target.max = r_static.max + dynamicDimensions / 2.f;

	if (RayVsRect(dynamicCenter, displacement, expanded_target, contact_point, contact_normal, contact_time))
		return contact_time >= 0.0f && contact_time < 1.0f;

	return false;
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
		else
			vel.y = 0.f;
	}

	glm::vec2 acc2D(0.f);

	// Input:
	if(glm::length(inputDir) > .001f)
		acc2D += glm::normalize(inputDir) * 50.f * (1.f + 1.f * !!(GetAsyncKeyState(VK_CONTROL)&0x8000));

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

	/************************
	 * COLLISION RESOLUTION *
	 ************************/

	onGround = false;

	glm::vec3 displacement = vel * dt;

	for(int i = 0; i < 3; i++) {
		AABB collider;

		{
			AABB movementBounds;
			movementBounds.min.x = std::min(pos.x, pos.x + vel.x * dt) - .3f;
			movementBounds.max.x = std::max(pos.x, pos.x + vel.x * dt) + .3f;

			movementBounds.min.y = std::min(pos.y, pos.y + vel.y * dt);
			movementBounds.max.y = std::max(pos.y, pos.y + vel.y * dt) + 1.8f;

			movementBounds.min.z = std::min(pos.z, pos.z + vel.z * dt) - .3f;
			movementBounds.max.z = std::max(pos.z, pos.z + vel.z * dt) + .3f;

			std::vector<AABB> initialColliders = world.getPossibleCollisions(movementBounds);

			float closestColliderTime = 1.f / 0.f;

			for(const AABB &col : initialColliders) {
				float t = 0;
				glm::vec3 cp, cn;
				// if(DynamicRectVsRect(vel, getCenter(), glm::vec3(.6f, 1.8f, .6f), dt, col, cp, cn, t)) {
				if(DynamicRectVsRect(displacement, getCenter(), glm::vec3(.6f, 1.8f, .6f), col, cp, cn, t)) {
					if(t >= 0.f && t < closestColliderTime) {
						collider = col;
						closestColliderTime = t;
					}
				}
			}

			if(closestColliderTime == 1.f / 0.f) // No collision found
				break;
		}


		glm::vec3 contact_point, contact_normal;
		float contact_time = 0.0f;
		// if (DynamicRectVsRect(vel, getCenter(), glm::vec3(.6f, 1.8f, .6f), dt, collider, contact_point, contact_normal, contact_time)) {
		if (DynamicRectVsRect(displacement, getCenter(), glm::vec3(.6f, 1.8f, .6f), collider, contact_point, contact_normal, contact_time)) {
			// DEBUG_RENRERER->box(collider.min, collider.max); // ####################   DEBUG   #####################

			if(contact_normal.x) {
				// pos.x += vel.x * (contact_time-.01) * (dt);
				pos.x += displacement.x * (contact_time-.01);
				vel.x = 0;
				displacement.x = 0;
			} else if(contact_normal.y) {
				// pos.y += vel.y * (contact_time-.01) * (dt);
				pos.y += displacement.y * (contact_time-.01);
				vel.y = 0;
				displacement.y = 0;
				if(contact_normal.y > 0)
					onGround = true;
			} else if(contact_normal.z) {
				// pos.z += vel.z * (contact_time-.01) * (dt);
				pos.z += displacement.z * (contact_time-.01);
				vel.z = 0;
				displacement.z = 0;
			}
		}
	}

	// Update the player rectangles position, with its modified velocity
	// pos += vel * dt;
	pos += displacement;

	DEBUG_RENRERER->box(getViewPos() + getViewDir() * 0.2f - glm::vec3(.0001f, .0001f, .0001f), getViewPos() + getViewDir() * .2f + glm::vec3(.0001f, .0001f, .0001f));

	if(perspective == Perspective::THIRD_PERSON)
		DEBUG_RENRERER->box(glm::vec3(pos.x-.3f, pos.y, pos.z-.3f), glm::vec3(pos.x+.3f, pos.y+1.8f, pos.z+.3f));


	// Object Picking:
	{
		const float MAX_DIST = 10.f;

		const glm::vec3 viewPos = getViewPos();
		const glm::vec3 viewDir = getViewDir();


		std::vector<AABB> initialColliders;
		{
			AABB bounds;
			bounds.min.x = std::min(pos.x, pos.x + viewDir.x * MAX_DIST) - .3f;
			bounds.max.x = std::max(pos.x, pos.x + viewDir.x * MAX_DIST) + .3f;

			bounds.min.y = std::min(pos.y, pos.y + viewDir.y * MAX_DIST);
			bounds.max.y = std::max(pos.y, pos.y + viewDir.y * MAX_DIST) + 1.8f;

			bounds.min.z = std::min(pos.z, pos.z + viewDir.z * MAX_DIST) - .3f;
			bounds.max.z = std::max(pos.z, pos.z + viewDir.z * MAX_DIST) + .3f;
			initialColliders = std::move(world.getPossibleCollisions(bounds));
		}

		float closestColliderTime = 1.f / 0.f;

		AABB collider;
		glm::vec3 colliderNormal;

		for(const AABB &col : initialColliders) {
			float t = 0;
			glm::vec3 cp, cn;
			if(RayVsRect(viewPos, viewDir*MAX_DIST, col, cp, cn, t)) {
				if(t >= 0.f && t < closestColliderTime) {
					collider = col;
					colliderNormal = cn;
					closestColliderTime = t;
				}
			}
		}

		if(closestColliderTime < 100) {
			// Block Breaking:
			if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
				const int chunkX = std::floor((collider.min.x + .5f) / 16.f);
				const int chunkZ = std::floor((collider.min.z + .5f) / 16.f);
				Chunk *const chunk = world.getChunk(chunkX, chunkZ);
				if(chunk != nullptr) {
					const int blockX = std::floor(collider.min.x + .5f) - chunkX * 16;
					const int blockY = std::floor(collider.min.y + .5f);
					const int blockZ = std::floor(collider.min.z + .5f) - chunkZ * 16;
					chunk->setBlock(world, blockX, blockY, blockZ, {BlockType::AIR});
				}
			}

			// Block Placement:
			static int ticksSinceLastPlacement = 0; ticksSinceLastPlacement++;
			if(GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
				if(ticksSinceLastPlacement > 120) {
					const int chunkX = std::floor((collider.min.x + colliderNormal.x + .5f) / 16.f);
					const int chunkZ = std::floor((collider.min.z + colliderNormal.z + .5f) / 16.f);
					Chunk *const chunk = world.getChunk(chunkX, chunkZ);
					if(chunk != nullptr) {
						const int blockX = std::floor(collider.min.x + colliderNormal.x + .5f) - chunkX * 16;
						const int blockY = std::floor(collider.min.y + colliderNormal.y + .5f);
						const int blockZ = std::floor(collider.min.z + colliderNormal.z + .5f) - chunkZ * 16;
						if(blockY >= 0 && blockY < 256) {
							chunk->setBlock(world, blockX, blockY, blockZ, {BlockType::GRASS});
							ticksSinceLastPlacement = 0;
						}
					}
				}
			} else
				ticksSinceLastPlacement = 200;

			// DEBUG_RENRERER->box(collider.min, collider.max); // highlight block at cursor
		}
	}
}

void Player::draw() const {
	bodyMesh.bind();
	bodyMesh.draw();
}







/**
 * Call the callback with (x,y,z,value,face) of all blocks along the line
 * segment from point 'origin' in vector direction 'direction' of length
 * 'radius'. 'radius' may be infinite.
 * 
 * 'face' is the normal vector of the face of that block that was entered.
 * It should not be used after the callback returns.
 * 
 * If the callback returns a true value, the traversal will be stopped.
 */
/*
int8_t signum(const float x);
int intbound(float s, float ds);
// void raycast(const glm::vec3 origin, const glm::vec3 direction, float radius, callback) {
bool raycast(World &world, const glm::vec3 origin, const glm::vec3 direction, float radius, glm::vec3 &result) {
	// From "A Fast Voxel Traversal Algorithm for Ray Tracing"
	// by John Amanatides and Andrew Woo, 1987
	// <http://www.cse.yorku.ca/~amana/research/grid.pdf>
	// <http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.42.3443>
	// Extensions to the described algorithm:
	//   • Imposed a distance limit.
	//   • The face passed through to reach the current cube is provided to
	//     the callback.

	// The foundation of this algorithm is a parameterized representation of
	// the provided ray,
	//                    origin + t * direction,
	// except that t is not actually stored; rather, at any given point in the
	// traversal, we keep track of the *greater* t values which we would have
	// if we took a step sufficient to cross a cube boundary along that axis
	// (i.e. change the integer part of the coordinate) in the variables
	// tMaxX, tMaxY, and tMaxZ.

	// Cube containing origin point.
	int x = std::floor(origin.x);
	int y = std::floor(origin.y);
	int z = std::floor(origin.z);

	// Break out direction vector.
	const float dx = direction.x;
	const float dy = direction.y;
	const float dz = direction.z;

	// Direction to increment x,y,z when stepping.
	const float stepX = signum(dx);
	const float stepY = signum(dy);
	const float stepZ = signum(dz);

	// See description above. The initial values depend on the fractional
	// part of the origin.
	int tMaxX = intbound(origin.x, dx);
	int tMaxY = intbound(origin.y, dy);
	int tMaxZ = intbound(origin.z, dz);

	// The change in t when taking a step (always positive).
	const float tDeltaX = stepX / dx;
	const float tDeltaY = stepY / dy;
	const float tDeltaZ = stepZ / dz;

	// Buffer for reporting faces to the callback.
	glm::vec3 face(0.f);

	// Avoids an infinite loop.
	if (dx == 0 && dy == 0 && dz == 0)
		return false;
		// throw new RangeError("Raycast in zero direction!");

	// Rescale from units of 1 cube-edge to units of 'direction' so we can
	// compare with 't'.
	radius /= std::sqrt(dx*dx + dy*dy + dz*dz);

	const int wx = 15;
	const int wy = 255;
	const int wz = 15;

	while (// ray has not gone past bounds of world
			(stepX > 0 ? x < wx : x >= 0) &&
			(stepY > 0 ? y < wy : y >= 0) &&
			(stepZ > 0 ? z < wz : z >= 0)) {

		// Invoke the callback, unless we are not *yet* within the bounds of the
		// world.
		// if (!(x < 0 || y < 0 || z < 0 || x >= wx || y >= wy || z >= wz))
		// 	if (callback(x, y, z, blocks[x*wy*wz + y*wz + z], face))
		// 		break;
		if (!(x < 0 || y < 0 || z < 0 || x >= wx || y >= wy || z >= wz)) {
			const Chunk *const chunk = world.getChunk(std::floor(x / 16.f), std::floor(z / 16.f));
			if(chunk == nullptr)
				break;
			const int blockX = x - std::floor(x / 16.f);
			const int blockZ = z - std::floor(z / 16.f);
			// if (callback(x, y, z, chunk->blocks[blockX][y][blockZ], face)) {
			if (chunk->getBlock(blockX, y, blockZ).type != BlockType::AIR) {
				DEBUG_RENRERER->box(glm::vec3(x, y, z), glm::vec3(x+1, y+1, z+1));
				return true;
			}
		}

		// tMaxX stores the t-value at which we cross a cube boundary along the
		// X axis, and similarly for Y and Z. Therefore, choosing the least tMax
		// chooses the closest cube boundary. Only the first case of the four
		// has been commented in detail.
		if (tMaxX < tMaxY) {
			if (tMaxX < tMaxZ) {
				if (tMaxX > radius)
					break;
				// Update which cube we are now in.
				x += stepX;
				// Adjust tMaxX to the next X-oriented boundary crossing.
				tMaxX += tDeltaX;
				// Record the normal vector of the cube face we entered.
				face.x = -stepX;
				face.y = 0;
				face.z = 0;
			} else {
				if (tMaxZ > radius)
					break;
				z += stepZ;
				tMaxZ += tDeltaZ;
				face.x = 0;
				face.y = 0;
				face.z = -stepZ;
			}
		} else {
			if (tMaxY < tMaxZ) {
				if (tMaxY > radius)
					break;
				y += stepY;
				tMaxY += tDeltaY;
				face.x = 0;
				face.y = -stepY;
				face.z = 0;
			} else {
				// Identical to the second case, repeated for simplicity in
				// the conditionals.
				if (tMaxZ > radius)
					break;
				z += stepZ;
				tMaxZ += tDeltaZ;
				face.x = 0;
				face.y = 0;
				face.z = -stepZ;
			}
		}
	}

	return false;
}

// Find the smallest positive t such that s+t*ds is an integer.
int intbound(float s, float ds) {
	constexpr auto mod = [](int value, int modulus) { return (value % modulus + modulus) % modulus; };

	if (ds < 0) {
		return intbound(-s, -ds);
	} else {
		s = fmodf(s, 1);
		// problem is now s+t*ds = 1
		return (1 - s) / ds;
	}
}

int8_t signum(const float x) {
	return (x > 0) ? 1 : (x < 0) ? -1 : 0;
}
*/