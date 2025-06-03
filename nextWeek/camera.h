#ifndef CAMERA_H
#define CAMERA_H

#include "frame.h"
#include "hittable.h"
#include "material.h"

class camera {
public:
	double aspect_ratio = 1.0;
	int image_width = 100;
	int samples_per_pixel = 10;
	int max_depth = 10;

	double vfov = 90; // Vertical field of view
	point3 lookfrom = point3(0, 0, 0);
	point3 lookat = point3(0, 0, -1);
	vec3 vup = vec3(0, 1, 0);

	double defocus_angle = 0;
	double focus_dist = 10;

	void render(const hittable& world) {
		initialize();

		// Render
		for (int j = 0; j < image_height; j++) {
			std::clog << "\rScanlines Remaining: " << (image_height - j) << ' ' << std::flush;
			for (int i = 0; i < image_width; i++) {
				Colour pixel_colour(0, 0, 0);
				for (int sample = 0; sample < samples_per_pixel; sample++) {
					ray r = get_ray(i, j);
					pixel_colour += ray_colour(r, max_depth, world);
				}

				frame->SetPixel(i, j, pixel_colour * pixel_samples_scale);
			}
		}

		std::clog << "\rDone.                          \n";

		frame->WriteFrame();
	};


private:
	int    image_height;        // Rendered image height
	double pixel_samples_scale; // Colour scale factor for a sum of pixel samples
	point3 center;              // Camera center
	point3 pixel00_loc;         // Location of pixel 0, 0
	vec3   pixel_delta_u;       // Offset to pixel to the right
	vec3   pixel_delta_v;       // Offset to pixel below
	vec3   u, v, w;             // Camera frame basis vectors
	vec3   defocus_disk_u;
	vec3   defocus_disk_v;
	Frame* frame;               // Image frame

	void initialize() {
		image_height = int(image_width / aspect_ratio);
		image_height = (image_height < 1) ? 1 : image_height;

		pixel_samples_scale = 1.0 / samples_per_pixel;

		center = lookfrom;

		// Determine viewport dimensions.
		auto theta = degrees_to_radians(vfov);
		auto h = std::tan(theta / 2);
		auto viewport_height = 2 * h * focus_dist;
		auto viewport_width = viewport_height * (double(image_width) / image_height);

		// Calculate the u,v,w unit basis vectos for the camera coordinate frame
		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);

		// Calculate the vectors across the horizontal and down the vertical viewport edges.
		auto viewport_u = viewport_width * u;   // Vector across viewport horizontal edge
		auto viewport_v = viewport_height * -v; // Vector down viewport vertical edge

		// Calculate the horizontal and vertical delta vectors from pixel to pixel.
		pixel_delta_u = viewport_u / image_width; 
		pixel_delta_v = viewport_v / image_height;

		// Calculate the location of the upper left pixel.
		auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
		pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

		// Calculate the camera defocus disc basis vectors
		auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
		defocus_disk_u = u * defocus_radius;
		defocus_disk_v = v * defocus_radius;

		frame = new Frame(image_width, image_height);
	};

	ray get_ray(int i, int j) {
		// Construct a camera ray originating from the defocus disk and directed at a randomly
		// sampled point around the pixel location i, j.

		auto offset = sample_square();
		auto pixel_sample = pixel00_loc
			                + ((i + offset.x()) * pixel_delta_u)
			                + ((j + offset.y()) * pixel_delta_v);

		auto ray_origin = defocus_angle <= 0 ? center : defocus_disk_sample();
		auto ray_direction = pixel_sample - ray_origin;

		return ray(ray_origin, ray_direction);
	}

	vec3 sample_square() const {
		// Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
		return vec3(random_double() - 0.5, random_double() - 0.5, 0);
	}

	point3 defocus_disk_sample() const {
		// Returns a random point in the camera defocus disk
		auto p = random_in_unit_disk();
		return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
	}

	Colour ray_colour(const ray& r, int depth, const hittable& world) const {
		if (depth <= 0) {
			return Colour(0, 0, 0);
		}
		hit_record rec;

		if (world.hit(r, interval(0.001, infinity), rec)) {
			ray scattered;
			Colour attenuation;
			if (rec.mat->scatter(r, rec, attenuation, scattered)) {
				return attenuation * ray_colour(scattered, depth - 1, world);
			}
			return Colour(0, 0, 0);
		}

		vec3 unit_direction = unit_vector(r.direction());
		auto a = 0.5 * (unit_direction.y() + 1.0);
		return (1.0 - a) * Colour(1.0, 1.0, 1.0) + a * Colour(0.5, 0.7, 1.0);
	}
};

#endif // CAMERA_H