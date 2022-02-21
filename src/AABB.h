#pragma once

#include "glm/glm.hpp"

class Ray {
public:
	glm::vec3 origin, dir;
	inline glm::vec3 at(const float t) const {
		return origin + t * dir;
	}
};

class AABB {
public:
	glm::vec3 min, max;

public:
	AABB();
	AABB(const glm::vec3 &min, const glm::vec3 &max);
	static AABB fromCenter(const glm::vec3 &center, const glm::vec3 &dimensions);

public:
	inline glm::vec3 dimensions() const {
		return max - min;
	}
	inline glm::vec3 center() const {
		return (min + max) / 2.f;
	}

public:
	bool collidesWith(const AABB &other) const;

	bool intersects(const Ray &ray, glm::ivec3 &contact_normal, float &t_hit_near) const;
};

bool DynamicRectVsRect(const glm::vec3 displacement, const AABB &dynamic, const AABB &r_static, glm::ivec3 &contact_normal, float &contact_time);