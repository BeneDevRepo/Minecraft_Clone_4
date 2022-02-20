#pragma once

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"

#include "OpenGL/StaticMesh.h"

#include "World.h"

#include <cstdint>

class Player {
private:
	glm::vec3 pos, vel;
	glm::vec2 inputDir;

	// euler Angles
    float Yaw, Pitch;

	float zoom;

	enum class Perspective : uint8_t { FIRST_PERSON=0, THIRD_PERSON=1 } perspective;
	bool flying;

	StaticMesh bodyMesh;

public:
	Player(const float x, const float y, const float z, const float FOV = 45.);
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
	inline glm::vec3 getFootPos() const {
		return this->pos;
	}
	inline glm::vec3 getViewPos() const {
		// return this->pos + glm::vec3(0.f, 28.f / 16.f, 0.f);
		return this->pos + glm::vec3(0.f, 1.62f, 0.f);
	}
	inline glm::vec3 getCenter() const {
		// return this->pos + glm::vec3(0.f, 28.f / 16.f, 0.f);
		return this->pos + glm::vec3(0.f, 1.8f / 2.f, 0.f);
	}
	inline glm::vec3 getRightHandPos() const {
		const float handAngle = glm::radians(Yaw + 45.f);
		glm::vec3 handDir(
			cos(handAngle),
			0.f,
			sin(handAngle));
		return getCenter() + handDir * .5f;
	}
	inline float getYaw() const {
		return Yaw;
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