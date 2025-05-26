# Ray Tracing in a Weekend Architecture

Written following  [_Ray Tracing in One Weekend_](https://raytracing.github.io/books/RayTracingInOneWeekend.html).

The main code base consists of nine header files, a main file and a
custom `Frame` class I added to to write EXR files directly from the 
renderer. The header files are as follows:

- `camera.h` Sets up the camera, image resolution and samples etc. as
  well as running the main rendering loop.
- `hittable.h` Base class for hittable objects. Defines the data
  structure for each ray/object intersection.
- `hitable_list.h` Defines a list of objects that make up the scene and
  test whether a given ray hits an object.
- `interval.h` Class fro basic interval testing (is a value inside the
  interval etc.).
- `material.h` Conatins the various different materials defined so far.
- `ray.h` Simple class defining a ray as an origin and a direction.
- `rtweekend.h` Contains various utility functions and includes.
- `sphere.h` Defines a sphere object, the only shape used in _Ray Tracing in a Weekend_.
- `vec3.h` Vector class.

## Camera
```
public:
    double aspect_ratio	    = 1.0              // Image aspect ratio
    int image_width         = 100              // Image width
    int samples_per_pixel   = 10               // Sample per pixel
    int max_depth           = 10               // Max number of bounces

    double vfov             = 90               // Vertical field of view
    point3 lookfrom         = point3(0, 0, 0)  // Camera position
    point3 lookat           = point3(0, 0, -1) // Vector camera is looking down
    vec3 vup                = vec3(0, 1, 0)    // Up vector

    double defocus_angle    = 0                // Effective amount of defocus
    double focus_dist       = 10               // Distance to focus point

    void render(const hittable& world)         // Main render loop

private:
    int    image_height;                       // Rendered image height
    double pixel_samples_scale;                // Colour scale factor for a sum of pixel samples
    point3 center;                             // Camera center
    point3 pixel00_loc;                        // Location of pixel 0, 0
    vec3   pixel_delta_u;                      // Offset to pixel to the right
    vec3   pixel_delta_v;                      // Offset to pixel below
    vec3   u, v, w;                            // Camera frame basis vectors
    vec3   defocus_disk_u;                     // Defocus disk up vector
    vec3   defocus_disk_v;                     // Defocus disk right vector
    Frame* frame;                              // Image frame

    void initialize()                          // Set up camera and renderer
    ray get_ray(int i, int j)                  // Generates a new starting ray for pixel (i, j)
    vec3 sample_square()                       // Returns a radom point in the +- 0.5 square
    point3 defocus_disk_sample()               // Returns a random point in a sphere radius 1.0
    Colour ray_colour(const ray& r, int depth, const hittable& world) // Recursively trace a ray around a scene
```

The camera class is the core of the render engine. It initilises the
renderer, starts firing rays into the scene and traces them around. It
contains a number of public variables that are the available settings
in the engine and has a single callable function `render()` which we
call from our `main` file. Image resolution is defined by the width and
the aspect raio.

### `initlise()`

The engine works by firing multiple rays through each pixel of the
image and averaginf their values together. To get each rays direction
we have to build a set of basis and delta vectors for the camera. The
basis vectors are 