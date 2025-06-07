#ifndef TEXTURE_H
#define TEXTURE_H

#include "rtweekend.h"
#include "rtw_openexr_image.h"

class texture {
public:
	virtual ~texture() = default;

	virtual Colour value(double u, double v, const point3& p) const = 0;
};

class solid_colour : public texture {
public:
	solid_colour(const Colour& albedo) : albedo(albedo) {};
	solid_colour(double red, double green, double blue) : solid_colour(Colour(red, green, blue)) {};

	Colour value(double u, double v, const point3& p) const override {
		return albedo;
	}


private:
	Colour albedo;
};

class checker_texture : public texture {
public:
	checker_texture(double scale, shared_ptr<texture> even, shared_ptr<texture> odd)
		: inv_scale(1.0 / scale), even(even), odd(odd) {
	};

	checker_texture(double scale, const Colour& c1, const Colour& c2)
		:checker_texture(scale, make_shared<solid_colour>(c1), make_shared<solid_colour>(c2)) {
	};

	Colour value(double u, double v, const point3& p) const override{
		auto xInteger = int(std::floor(inv_scale * p.x()));
		auto yInteger = int(std::floor(inv_scale * p.y()));
		auto zInteger = int(std::floor(inv_scale * p.z()));

		bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

		return isEven ? even->value(u, v, p) : odd->value(u, v, p);
	};


private:
	double inv_scale;
	shared_ptr<texture> even;
	shared_ptr<texture> odd;

};

class image_texture :public texture {
public:
	image_texture(const char* filename) : image(filename) {};

	Colour value(double u, double v, const point3& p) const override{
		if (image.height() <= 0) return Colour(0, 1, 1);

		// Clamp input texture coordinates to [0,1] x [1,0]
		u = interval(0, 1).clamp(u);
		v = 1.0 - interval(0, 1).clamp(v);  // Flip V to image coordinates

		auto i = int(u * image.width());
		auto j = int(v * image.height());
		
		return image.pixel_colour(i, j);
	}


private:
	rtw_image image;
};

#endif // TEXTURE_H
