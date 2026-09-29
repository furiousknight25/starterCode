#pragma once

#include "shape.h"

class Triangle : public Shape {
public:
    point3 v0{0.0, 0.0, 0.0};
    point3 v1{1.0, 0.0, 0.0};
    point3 v2{0.0, 1.0, 0.0};

    Triangle() = default;

    Triangle(const point3& p0, const point3& p1, const point3& p2, const color& col = color(0.2, 0.8, 0.3))
        : Shape((p0 + p1 + p2) / 3.0, vec3(0, 0, 0), col), v0(p0), v1(p1), v2(p2) {}

    vec3 normal() const {
        return unit_vector(cross(v1 - v0, v2 - v0));
    }

    point3 vertex(int i) const {
        if (i == 0) return v0;
        if (i == 1) return v1;
        return v2;
    }

    bool intersect(const ray &r) const override;
    bool intersect(const ray &r) override {
        return static_cast<const Triangle*>(this)->intersect(r);
    }

    bool intersect(const ray &r, double t_min, double t_max, hit_record &rec) const override;
    bool intersect(const ray &r, interval ray_t, hit_record &rec) const override {
        return intersect(r, ray_t.min, ray_t.max, rec);
    }
};
