#include "AABB.h"

AABB::AABB():
	min(0.f), max(0.f) {
}

AABB::AABB(const glm::vec3 &min, const glm::vec3 &max):
	min(min), max(max) {
}

AABB AABB::fromCenter(const glm::vec3 &center, const glm::vec3 &dimensions) {
	return AABB{center - dimensions / 2.f, center + dimensions / 2.f};
}




bool AABB::collidesWith(const AABB &other) const {
	return min.x < other.max.x && max.x > other.min.x
		&& min.y < other.max.y && max.y > other.min.y
		&& min.z < other.max.z && max.z > other.min.z;
}


bool AABB::intersects(const Ray &ray, glm::ivec3 &contact_normal, float &t_hit_near) const {
	// Calculate intersections with rectangle bounding axes
	glm::vec3 t_near = (this->min - ray.origin) / ray.dir;
	glm::vec3 t_far = (this->max - ray.origin) / ray.dir;

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

	// --- Dimensional extrapolation:
	contact_normal = { 0.f, 0.f, 0.f };

	if (t_near.x > t_near.y && t_near.x > t_near.z) {
		if (ray.dir.x < 0)
			contact_normal = { 1, 0, 0 };
		else
			contact_normal = { -1, 0, 0 };
	} else if (t_near.y > t_near.x && t_near.y > t_near.z) {
		if (ray.dir.y < 0)
			contact_normal = { 0, 1, 0 };
		else
			contact_normal = { 0, -1, 0 };
	} else if (t_near.z > t_near.x && t_near.z > t_near.y) {
		if (ray.dir.z < 0)
			contact_normal = { 0, 0, 1 };
		else
			contact_normal = { 0, 0, -1 };
	}

	// Note if t_near == t_far, collision is principly in a diagonal
	// so pointless to resolve. By returning a CN={0,0} even though its
	// considered a hit, the resolver wont change anything.

	return true;
}

bool DynamicRectVsRect(const glm::vec3 displacement, const AABB &dynamic, const AABB &r_static, glm::ivec3 &contact_normal, float &contact_time) {
	// Reject if displacement is 0:
	if (displacement.x == 0 && displacement.y == 0 && displacement.z == 0) return false;

	// Expand target rectangle by source dimensions
	const AABB expanded_target = AABB::fromCenter(r_static.center(), r_static.dimensions() + dynamic.dimensions());

	if (expanded_target.intersects(Ray{ dynamic.center(), displacement }, contact_normal, contact_time))
		return contact_time >= 0.0f && contact_time < 1.0f;

	return false;
}