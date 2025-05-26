#ifndef FRAME_H
#define FRAME_H

#include <OpenEXR/ImfRgbaFile.h>
#include <cassert>
#include <Imath/half.h>

#include "rtweekend.h"


class Frame
{
public:
	Frame(const int x, const int y, const Imf::Rgba colour=Imf::Rgba(1.0, 1.0, 1.0)) :
		x_resolution_(x),
		y_resolution_(y),
		clear_colour_(colour)
	{
		rgba_data_ = new Imf::Rgba[x_resolution_ * y_resolution_];

		for (int i = 0; i < x_resolution_ * y_resolution_; i++) {
			rgba_data_[i] = clear_colour_;
		}
	}

	bool WriteFrame(const char* filename="out.exr")
	{
		try {
			Imf::RgbaOutputFile out(filename, x_resolution_, y_resolution_, Imf::WRITE_RGBA);
			out.setFrameBuffer(rgba_data_, 1, x_resolution_);
			out.writePixels(y_resolution_);

			return true;
		}
		catch (const std::exception& e) {
			std::cout << "Failed to write frame: " << e.what();

			return false;
		}
	}

	void ClearFrame()
	{
		for (int i = 0; i < x_resolution_ * y_resolution_; i++) {
			rgba_data_[i] = clear_colour_;
		}
	}

	void SetPixel(const int x, const int y, const Imf::Rgba colour)
	{
		assert(x >= 0);
		assert(x < x_resolution_);
		assert(y >= 0);
		assert(y < y_resolution_);

		int flip_y = y;// y_resolution_ - 1 - y;

		rgba_data_[x_resolution_ * flip_y + x] = colour;
	}

	void SetPixel(const int x, const int y, const Colour colour)
	{
		static const interval intensity(0.000, 0.999);
		Imf::Rgba col;
		col.r = intensity.clamp(colour.x());
		col.g = intensity.clamp(colour.y());
		col.b = intensity.clamp(colour.z());
		col.a = 1.0;

		SetPixel(x, y, col);
	}

private:
	int x_resolution_;
	int y_resolution_;

	Imf::Rgba* rgba_data_;
	Imf::Rgba clear_colour_;
};



#endif // FRAME_H