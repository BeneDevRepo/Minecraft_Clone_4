#pragma once

#include <cmath>
#include <iostream>
#include <random>
#include <atomic>

// using std::sqrt;

// inline double clamp(double x, double min, double max) {
// 	if (x < min) return min;
// 	if (x > max) return max;
// 	return x;
// }

inline double random_double() {
	static std::atomic_uint32_t seed = 0;
	/*thread_local*/ static std::mt19937 generator(seed.fetch_add(13)*0);
	/*thread_local*/ static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	return distribution(generator);
}

// inline double random_double() {
// 	return rand() / (RAND_MAX + 1.0);// Returns a random real in [0,1).
// }

inline double random_double(double min, double max) {
	return min + (max-min)*random_double(); // Returns a random real in [min,max).
}

class vec3 {
public:
	double e[3];
	
public:
	vec3() : e{0, 0, 0} {}
	vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

	inline double x() const { return e[0]; }
	inline double y() const { return e[1]; }
	inline double z() const { return e[2]; }

	inline vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
	inline double operator[](int i) const { return e[i]; }
	inline double& operator[](int i) { return e[i]; }

	inline vec3& operator+=(const vec3 &v) {
		e[0] += v.e[0];
		e[1] += v.e[1];
		e[2] += v.e[2];
		return *this;
	}

	inline vec3& operator*=(const double t) {
		e[0] *= t;
		e[1] *= t;
		e[2] *= t;
		return *this;
	}

	inline vec3& operator/=(const double t) {
		return *this *= 1/t;
	}

	inline double length() const {
		return std::sqrt(length_squared());
	}

	inline double length_squared() const {
		return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
	}
};

// Type aliases for vec3
using point3 = vec3;   // 3D point
using color = vec3;    // RGB color


// vec3 Utility Functions

inline std::ostream& operator<<(std::ostream &out, const vec3 &v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(const double t, const vec3 &v) {
    return vec3(t*v.e[0], t*v.e[1], t*v.e[2]);
}

inline vec3 operator*(const vec3 &v, const double t) {
    return t * v;
}

inline vec3 operator/(const vec3 &v, const double t) {
    return (1/t) * v;
}

inline double dot(const vec3 &u, const vec3 &v) {
    return u.e[0] * v.e[0]
         + u.e[1] * v.e[1]
         + u.e[2] * v.e[2];
}

inline vec3 cross(const vec3 &u, const vec3 &v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(const vec3 &v) {
    return v / v.length();
}

inline double length_squared(const vec3 &vec) {
	return dot(vec, vec);
}

inline bool near_zero(const vec3& v) {
	constexpr double s = 1e-8;
	return fabs(v.x()) < s && fabs(v.x()) < s && fabs(v.x()) < s;
}

inline vec3 reflect(const vec3& v, const vec3& n) {
	return v - (n * (2. * dot(v, n)));
}

inline vec3 lerp(const vec3& a, const vec3& b, const double t) {
	return (1-t) * a + t * b;
}

// Random

inline vec3 random() {
	return vec3(random_double(), random_double(), random_double());
}

inline vec3 random(const double min, const double max) {
	return vec3(random_double(min,max), random_double(min,max), random_double(min,max));
}

inline vec3 random_in_unit_sphere() {
	while (true) {
		const vec3 p = random(-1, 1);
		if (p.length_squared() < 1)
			return p;
	}
}

inline vec3 random_in_unit_disk() {
    while (true) {
        const vec3 p(random_double(-1,1), random_double(-1,1), 0);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

inline vec3 random_unit_vector() {
	return unit_vector(random_in_unit_sphere());
}

inline vec3 random_in_hemisphere(const vec3& normal) {
	const vec3 in_unit_sphere = random_in_unit_sphere();
	if (dot(in_unit_sphere, normal) > 0.0) // In the same hemisphere as the normal
		return in_unit_sphere;
	else
		return -in_unit_sphere;
}