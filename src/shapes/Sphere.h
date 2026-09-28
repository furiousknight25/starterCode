#pragma once

#include "shape.h"

class Sphere : public Shape {
public:
    double radius = 1.0;

    Sphere() = default;
    Sphere(const point3 &center, double r) : Shape(center), radius(r) {}
    Sphere(const point3 &center, double r, const vec3 &rot) : Shape(center, rot), radius(r) {}

    const point3& center() const { return position; }
    point3& center() { return position; }
    double get_radius() const { return radius; }
    void set_radius(double r) { radius = r; }

    bool intersect(const ray &r) const override;
    bool intersect(const ray &r) override {
        return static_cast<const Sphere*>(this)->intersect(r);
    }
};
