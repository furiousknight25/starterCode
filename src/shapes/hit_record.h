#pragma once

#include "../math/vec3.h"
#include "../math/ray.h"
#include <memory>

class Shader; // forward declaration so we can store a pointer to the shader

struct hit_record {
    point3 p{0.0, 0.0, 0.0};
    vec3 normal{0.0, 0.0, 1.0};
    double t{0.0};
    bool front_face{true};
    color mat_color{0.8, 0.8, 0.8};
    bool is_sphere{false};

    // View direction points back towards the camera/eye from hit point
    vec3 view_dir{0.0, 0.0, 1.0};

    // Pointer to shader of the shape we hit
    std::shared_ptr<Shader> shader{nullptr};

    // Sets hit record normal vector:
    // outward_normal is assumed to have unit length.
    void set_face_normal(const ray& r, const vec3& outward_normal) {
        // view direction is just pointing back along ray to the eye!
        view_dir = unit_vector(-r.direction());
        front_face = dot(r.direction(), outward_normal) < 0.0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// lab asks for HitStructure
using HitStructure = hit_record;
