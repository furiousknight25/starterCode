#pragma once

#include "../math/vec3.h"
#include "../math/ray.h"

struct hit_record {
    point3 p{0.0, 0.0, 0.0};
    vec3 normal{0.0, 0.0, 1.0};
    double t{0.0};
    bool front_face{true};
    color mat_color{0.8, 0.8, 0.8};
    bool is_sphere{false};

    // Sets hit record normal vector:
    // outward_normal is assumed to have unit length.
    void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0.0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};
