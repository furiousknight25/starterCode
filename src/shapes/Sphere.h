#pragma once

#include "shape.h"

class Sphere : public Shape {
public:
    double radius = 1.0;
    bool use_normal_coloring = false;

    Sphere() = default;
    Sphere(const point3 &center, double r)
        : Shape(center, vec3(0, 0, 0), color(0.8, 0.2, 0.2)), radius(r) {}
    Sphere(const point3 &center, double r, const color& col)
        : Shape(center, vec3(0, 0, 0), col), radius(r) {}
    Sphere(const point3 &center, double r, const vec3 &rot, const color& col)
        : Shape(center, rot, col), radius(r) {}

    const point3& center() const { return position; }
    point3& center() { return position; }
    double get_radius() const { return radius; }
    void set_radius(double r) { radius = r; }

    bool intersect(const ray &r) const override;
    bool intersect(const ray &r) override {
        return static_cast<const Sphere*>(this)->intersect(r);
    }

    bool intersect(const ray &r, double t_min, double t_max, hit_record &rec) const override;
    bool intersect(const ray &r, interval ray_t, hit_record &rec) const override {
        return intersect(r, ray_t.min, ray_t.max, rec);
    }
};
