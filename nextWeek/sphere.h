#ifndef SPHERE_H
#define SPHERE_H

#include "rtweekend.h"
#include "hittable.h"

class sphere : public hittable {
public:
	// Stationary sphere
	sphere(const point3& static_center, double radius, shared_ptr<material> mat)
		: center_(static_center, vec3(0,0,0)), radius_(std::fmax(0, radius)), mat(mat) {}

	// Moving sphere
	sphere(const point3& center1, const point3& center2, double radius, shared_ptr<material> mat)
	    : center_(center1, center2 - center1), radius_(std::fmax(0, radius)), mat(mat) {}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		point3 current_center = center_.at(r.time());
		vec3 oc = current_center - r.origin();
		auto a = r.direction().length_squared();
		auto h = dot(r.direction(), oc);
		auto c = oc.length_squared() - radius_ * radius_;
		auto discriminant = h * h - a * c;

		if (discriminant < 0) {
			return false;
		}

		auto sqrtd = std::sqrt(discriminant);

		auto root = (h - sqrtd) / a;
		if (!ray_t.surrounds(root)) {
			root = (h + sqrtd) / a;
			if (!ray_t.surrounds(root)) {
				return false;
			}
		}

		rec.t = root;
		rec.p = r.at(rec.t);
		vec3 outward_normal = (rec.p - current_center) / radius_;
		rec.set_face_normal(r, outward_normal);
		rec.mat = mat;

		return true;
	}


private:
	ray center_;
	double radius_;
	shared_ptr<material> mat;
};

#endif // SPHERE_H
