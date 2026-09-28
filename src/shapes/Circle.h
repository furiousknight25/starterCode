#pragma once //hmm what does pragma mean again

#include "Sphere.h"

// Circle inherits from Sphere (representing a 3D sphere that projects to a circle in 2D)
class Circle : public Sphere {
    public:
        Circle() = default;
        Circle(const vec3 &center, double r) : Sphere(center, r) {}
        Circle(const vec3 &center, double r, const vec3 &rot) : Sphere(center, r, rot) {}
};