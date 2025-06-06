#ifndef SPHERE_H
#define SPHERE_H

#include "rtweekend.h"
#include "hittable.h"

class sphere : public hittable {
public:
	// Stationary sphere
	sphere(const point3& static_center, double radius, shared_ptr<material> mat)
		: center_(static_center, vec3(0,0,0)), radius_(std::fmax(0, radius)), mat(mat)
	{
		auto rvec = vec3(radius, radius, radius);
		bbox = aabb(static_center - rvec, static_center + rvec);
	}

	// Moving sphere
	sphere(const point3& center1, const point3& center2, double radius, shared_ptr<material> mat)
	    : center_(center1, center2 - center1), radius_(std::fmax(0, radius)), mat(mat)
	{
		auto rvec = vec3(radius, radius, radius);
		aabb box1(center_.at(0) - rvec, center_.at(0) + rvec);
		aabb box2(center_.at(1) - rvec, center_.at(1) + rvec);
		bbox = aabb(box1, box2);
	}

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
		get_sphere_uv(outward_normal, rec.u, rec.v);
		rec.mat = mat;

		return true;
	}

	aabb bounding_box() const override { return bbox; };


private:
	ray center_;
	double radius_;
	shared_ptr<material> mat;
	aabb bbox;

	static void get_sphere_uv(const point3& p, double& u, double& v) {
		// p: a given point on the sphere of radius one, centered at the origin.
		// u: returned value [0,1] of angle around the Y axis from X=-1.
		// v: returned value [0,1] of angle from Y=-1 to Y=+1.
		//     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
		//     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
		//     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

		auto theta = std::acos(-p.y());
		auto phi = std::atan2(-p.z(), p.x()) + pi;

		u = phi / (2 * pi);
		v = theta / pi;
	}
};

#endif // SPHERE_H
