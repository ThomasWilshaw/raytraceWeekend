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

### `initilise()`

The engine works by firing multiple rays through each pixel of the
image and averaging their values together. To get each rays direction
we have to build a set of basis and delta vectors for the camera. The
basis vectors define the plane that is perpendicular to the vector
between the camera and the lookat point. Wel also then calculate the
pixel delta vector which is the spacing between each pixel.

### `get_ray()`

Returns a ray that starts at the cameras origin and passes through the
`(ith, jth)` pixel plus/minus a small amount. Thsi plus minus is what
allows us to antiailiase the pixel.

### `ray_colour()`

Takes a ray and tests it against every object in the scene. If it hits
an object we process its material and recursivaly test the bounced ray
and if it does not we return our sky gradient. If the object has no
material assigned it returns black and bounces no more rays.

Also takes a depth argument so we can control how many bounces a ray
can make.

### `render()`

Initilises the scene and then loops through each pixel generating `n`
rays per pixel and averageing them. The results is them written to
the current frame.


## Hittable

This is a pure abstract class that all objects that a ray can
intersect with are derived from. It has single function `hit()`
which takes a ray and tests whether it has hit the object.

```
public:
    virtual bool hit(const ray& r, interval ray_t, hit_record& rec) const = 0;
```

The file also defines the `hit_record` class as simple data structure
that stores the location of a ray/object intersection and any other
data that goes along with that intersection. The class also defines one
function `set_face_normal()` which sets the `normal` value.

```
public:
    point3 p                  // Location of ray/object intersection
    vec3 normal               // Normal vector of object at intersection point
    shared_ptr<material> mat  // Object material at intersection point
    double t                  // Parametric distance along the ray 
    bool front_face           // Whether we have hit a front facing surface or not

    void set_face_normal(const ray& r, const vec3& outward_normal) // Sets face normal
```

## Hittable List

```
public:
    std::vector<shared_ptr<hittable>> objects                // List of hittable objects

    hittable_list()                                          // Creates a new empty list
    hittable_list(shared_ptr<hittable> object)               // Creates a new list with an item added
    void clear()                                             // Clears a list
    void add(shared_ptr<hittable> object)                    // Adds a hittable object to the list

    bool hit(const ray& r, interval ray_t, hit_record& rec)  // Overriden function that tests if a ray
                                                             // hit an object in the list
```

This class holds a list of all the hittable objects in a scene and
allows us to add to or clear said list. It also implements the virtual
class `hittable` and so defines the function `hit(...)` which interates
through our list of objects and calls each ones `hit()` functiopn with
the given ray. The ray is tested against every object in the scene and
a record is kept of the currently closest intersection found. Only the
closest intersection is returned.

## Interval

```
public:
    double min, max;

    interval() : min(+infinity), max(-infinity) {}            // Default interval is empty
    interval(double min, double max) : min(min), max(max) {}  // Create a interval between min and max

    double size()                                             // Return the size of an interval
    bool contains(double x)                                   // Tests if min <= x <= max
    bool surrounds(double x)                                  // Tests if min < x < max
    double clamp(double x)                                    // Clamps a value to the bounds of the interval
```

Intervals are defined as a gap between a minimum and a maximum value.
This class also implements functions to work with intervals such as
testing if a value falls within an interval or clamping a value to an
interval. We also define two constant intervals, `empty` and
`universe`.

```
const interval interval::empty = interval(+infinity, -infinity);
const interval interval::universe = interval(-infinity, +infinity);
```

The main use of intervals is in `hittable_list::hit()`. We start with
an interval between `0.001` and `inf` and for every object that is hit,
if the hit point is within the current interval, the interval shrinks
to `0.001` and `hit_point`, otherwise it is skipped.

## Material

```
public:
	virtual ~material() = default;

	virtual bool scatter( const ray& r_in, const hit_record& rec, Colour& attenuation, ray& scattered)
```

This is a virtual class that all materials inherit from and it
consists of a single function `scatter(...)`. It takes an incoming ray,
an hit record that contains information about where the ray hit, a
colour attenuation to pass back up and a new ray that will be the
scattered outgoing ray.

To use the `lambertian` class as an example, we take the hit points
normal vector and use that to generate a random outward vector that
becomes the direction vector for our new ray (more complex shaders
such as the `dielectric` use the incoming ray and normal to
calculate a reflection or a refraction ray). The albedo colour of the
shader is passed up and used to attenuate the current ray colour. For
example if the aledo is `Colour(1,0,0)` the ray colour will be fully
attenuated in the green and blue channels and uneffected in the red as
it the object absorbed all but the red light.

## Ray

```
public:
	ray()
	ray(const point3& origin, const vec3& direction)

	const point3& origin()
	const vec3& direction()

	point3 at(double t)

private:
	point3 orig_;
	vec3 dir_;
};
```

Basic ray object, rays are stored as a origin point and a direction,
all in three dimensions. ALso provides and `at()` function that
interpolates along a ray by the amount `t`.

## RTWeekend

```
// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility Functions

inline double degrees_to_radians(double degrees)

inline double random_double()

inline double random_double(double min, double max)
```

Utility header that defines several constants and commonly used
funcitons. Also includes several commonly used headers.

## Sphere

The only mesh/object defined so far, it ihnherits from the `hittable`
and so defines the function `hit(...)`. Spheres are defined with an
origin and a radius and from this we can easily calculate if a ray
has intersected with it or not. This class is also assigned a
material.

## Vec3

Defines a three dimensional vector class and overloads various
operators to make woring with them easier. Also includes a set of
vector specific functions such as `legnth()`, `cross()` etc.
