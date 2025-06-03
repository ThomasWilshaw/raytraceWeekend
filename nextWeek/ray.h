#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
public:
	ray() {}
	ray(const point3& origin, const vec3& direction, double time)
		: orig_(origin), dir_(direction), tm(time) {}

	ray(const point3& origin, const vec3& direction)
		: ray(origin, direction, 0) {}

	const point3& origin() const { return orig_; };
	const vec3& direction() const { return dir_; };

	double time() const { return tm; };

	point3 at(double t) const {
		return orig_ + t * dir_;
	};

private:
	point3 orig_;
	vec3 dir_;
	double tm;
};

#endif // RAY_H