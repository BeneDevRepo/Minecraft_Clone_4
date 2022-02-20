#pragma once

#include "RayTracing/vec3.h"
#include "RayTracing/Ray.h"
// #include "hit_record.h"

class hit_record;

class Material {
public:
	virtual bool scatter(const Ray& r_in, const hit_record& rec, color& attenuation, Ray& scattered) const = 0;
};