#pragma once

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "OpenGL/StaticMesh.h"

#include "World.h"
#include "AABB.h"
#include "Coordinates.h"

#include <cstdint>


class Player {
private:
	// ChunkPos chunkPos; // Coordinates of Chunk the Player is currently in
	// glm::vec3 pos; // Position of player relative to chunk
	EntityPos pos; // Position of player relative to chunk
	// ChunkPos virtualOrigin;
	glm::vec3 vel;
	glm::vec2 inputDir;

	// euler Angles
    float Yaw, Pitch;

	float zoom;

	enum class Perspective : uint8_t { FIRST_PERSON=0, THIRD_PERSON=1 } perspective;
	bool flying;

	StaticMesh bodyMesh;

public:
	// Player(const float x, const float y, const float z, const float FOV = 45.);
	Player(const ChunkPos &chunkPos, const glm::vec3 &relativePos, const float FOV = 45.);
	Player(const glm::vec3 &absPos, const float FOV = 45.);
	~Player();

	Player(const Player&) = delete;
	Player &operator=(const Player&) = delete;
	Player(Player&&) = delete;
	Player &operator=(Player&&) = delete;

	void handleMouseInput(const float dt, const int mouseX, const int mouseY);
	void update(const float dt, World& world);
	void draw() const;

	inline float getFOV() const {
		return zoom;
	}
	inline float getYaw() const { // rotation around y
		return Yaw;
	}
	static constexpr glm::vec3 getDimensions() {
		return glm::vec3(.6f, 1.8f, .6f);
	}

	inline glm::vec3 getFootPos() const {
		// return this->pos;
		// return this->pos.computeAbsolute();
		return pos.relativePos();
	}
	inline ChunkPos getVirtualOrigin() const {
		// return {0, 0, 0};
		return pos.chunkPos();
	}


	inline glm::vec3 getViewPos() const {
		return getFootPos() + glm::vec3(0.f, 1.62f, 0.f);
	}
	inline glm::vec3 getCenter() const {
		return getFootPos() + glm::vec3(0.f, getDimensions().y / 2.f, 0.f);
	}

	inline Ray getViewRay() const {
		return Ray{ getViewPos(), getViewDir() };
	}
	inline AABB getAABB() const {
		return AABB::fromCenter(getCenter(), getDimensions());
	}

	inline glm::vec3 getRightHandPos() const {
		const float handAngle = glm::radians(Yaw + 45.f);
		glm::vec3 handDir(
			cos(handAngle),
			0.f,
			sin(handAngle));
		return getCenter() + handDir * .5f;
	}

	inline glm::vec3 getViewDir() const {
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		return glm::normalize(front);
	}
	inline glm::mat4 getViewMatrix() const {
		static constexpr glm::vec3 UP = glm::vec3{0.f, 1.f, 0.f};
		switch(perspective) {
			case Perspective::FIRST_PERSON: return glm::lookAt(getViewPos(), getViewPos() + getViewDir(), UP);
			case Perspective::THIRD_PERSON: return glm::lookAt(getViewPos() - getViewDir() * 3.f, getViewPos(), UP);
		}
		printf("Catastrophic Failiure in Player::getViewMatrix()\n");
		return *(glm::mat4*)nullptr; // XD
    }
	inline bool bodyVisible() const {
		switch(perspective) {
			case Perspective::FIRST_PERSON: return false;
			case Perspective::THIRD_PERSON: return true;
		}
		return false;
	}
};