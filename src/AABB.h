#pragma once

#include "glm/glm.hpp"

class AABB {
public:
	glm::vec3 min, max;

public:
	bool collides(const AABB &other) const;
};