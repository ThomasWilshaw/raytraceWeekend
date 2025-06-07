#ifndef RTW_STB_IMAGE_H
#define RTW_STB_IMAGE_H

#include <cstdlib>
#include <iostream>

#include <OpenEXRConfig.h>
#include <ImfRgbaFile.h>
#include <ImfStringAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfArray.h>
#include <OpenEXR/ImfFloatAttribute.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfIntAttribute.h>
#include <OpenEXR/ImfOutputFile.h>
#include <OpenEXR/ImfChannelList.h>

#include <ImfNamespace.h>

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace IMATH_NAMESPACE;

class rtw_image {
public:
    rtw_image(const char* image_filename) {
        // Loads image data from the specified file. If the RTW_IMAGES environment variable is
        // defined, looks only in that directory for the image file. If the image was not found,
        // searches for the specified image file first from the current directory, then in the
        // images/ subdirectory, then the _parent's_ images/ subdirectory, and then _that_
        // parent, on so on, for six levels up. If the image was not loaded successfully,
        // width() and height() will return 0.

        auto filename = std::string(image_filename);
        auto imagedir = getenv("RTW_IMAGES");

        // Hunt for the image file in some likely locations.
        if (imagedir && load((std::string(imagedir) + "/" + image_filename).c_str())) return;
        if (load(filename.c_str())) return;

        std::cerr << "ERROR: Could not load image file '" << image_filename << "'.\n";
    }

    ~rtw_image() {
    }

    bool load(const char* filename)
    {
        RgbaInputFile file(filename);
        Box2i dw = file.dataWindow();

        image_width = dw.max.x - dw.min.x + 1;
        image_height = dw.max.y - dw.min.y + 1;

        if (image_width == 0) {
            return false;
        }

        pixels.resizeErase(image_height, image_width);

        file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * image_width, 1, image_width);
        file.readPixels(dw.min.y, dw.max.y);

        return true;
    }

    int width()  const { return image_width; }
    int height() const { return image_height; }

    const Rgba pixel_data(int x, int y) const {
        // Return the address of the RGB/A bytes of the pixel at x,y. If there is no image
        // data, returns magenta.
        static Rgba magenta = Rgba(255, 0, 255);
        if (image_width == 0) return magenta;

        x = clamp(x, 0, image_width);
        y = clamp(y, 0, image_height);

        return pixels[y][x];
    }

    const Colour pixel_colour(int x, int y) const
    {
        Rgba data = pixel_data(x, y);
        Colour col(data.r, data.g, data.b);

        return col;
    }


private:
    int      bytes_per_pixel = 3;
    int            image_width = 0;         // Loaded image width
    int            image_height = 0;        // Loaded image height
    int            bytes_per_scanline = 0;
    Array2D<Rgba> pixels;

    static int clamp(int x, int low, int high) {
        // Return the value clamped to the range [low, high).
        if (x < low) return low;
        if (x < high) return x;
        return high - 1;
    }

};


#endif // RTW_STB_IMAGE_H
