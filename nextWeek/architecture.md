# Ray Tracing Next Week Architecture

This document is sllightly different to the Weekend document in that
it only covers changes to the previous body of code rather than the 
whole thing. It is grouped into the same sections as the book.

## Motion Blur
A complete motion blur system would allow for everything from a moving
camera to shutter speeds, frame rates and moving objects. To keep things
simple we only implement moving objects. For each sample at each pixel
we now also assign it a random point in time between zero and one.
Object centers are now stored as a ray and are given two
center points, one for the start of their motion and one for the end.
We can use our ray time value to interpolate between these two
centers and use this as the object origin for our ray intersection
tests. The time value assigned to a ray is passed along to the new
scatter ray when there is a ray/object intersection.
