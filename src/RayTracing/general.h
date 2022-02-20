#pragma once

#include "RayTracing/vec3.h"
#include "RayTracing/color.h"
#include "RayTracing/Ray.h"
#include "RayTracing/Objects/hittable_list.h"

color ray_color(const Ray& r, const hittable& world, const int depth) {
	if (depth <= 0)
		return color(0, 0, 0);

	const double INF = 1. / 0.;

	hit_record rec;
	if (world.hit(r, 0.00001, INF, rec)) {
		Ray scattered;
		color attenuation;
		if (rec.material->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth-1);
		return color(0, 0, 0);
	}

	vec3 unit_direction = unit_vector(r.dir);
	float t = 0.5 * (unit_direction.y() + 1.0);
	return lerp(color(1.0, 1.0, 1.0), color(0.5, 0.7, 1.0), t);
}