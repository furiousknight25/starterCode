#include "Sphere.h"
#include <cmath>

bool Sphere::intersect(const ray& r) const {
    vec3 oc = r.origin() - position;
    auto a = dot(r.direction(), r.direction());
    if (a <= 1e-12) {
        return false;
    }
    auto b = 2.0 * dot(r.direction(), oc);
    auto c = dot(oc, oc) - (radius * radius);
    auto discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) {
        return false;
    }

    // A ray travels forward along its direction (t >= 0).
    // The roots of a * t^2 + b * t + c = 0 are:
    // t = (-b +/- sqrt(discriminant)) / (2 * a)
    // Since a > 0, the larger root is (-b + sqrt(discriminant)) / (2 * a).
    // If the largest root is negative, the entire sphere is strictly behind the ray origin.
    auto sqrt_d = std::sqrt(discriminant);
    auto t_max = (-b + sqrt_d) / (2.0 * a);
    return (t_max >= 0.0);
}

bool Sphere::intersect(const ray& r, double t_min, double t_max, hit_record& rec) const {
    vec3 oc = r.origin() - position;
    auto a = dot(r.direction(), r.direction());
    if (a <= 1e-12) {
        return false;
    }
    auto b = 2.0 * dot(r.direction(), oc);
    auto c = dot(oc, oc) - (radius * radius);
    auto discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) {
        return false;
    }

    auto sqrt_d = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range [t_min, t_max]
    auto root = (-b - sqrt_d) / (2.0 * a);
    if (root < t_min || root > t_max) {
        root = (-b + sqrt_d) / (2.0 * a);
        if (root < t_min || root > t_max) {
            return false;
        }
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outward_normal = unit_vector(rec.p - position);
    rec.set_face_normal(r, outward_normal);
    rec.is_sphere = true;
    rec.shader = shader; // give the hit record the sphere's shader
    if (use_normal_coloring) {
        rec.mat_color = 0.5 * (rec.normal + color(1.0, 1.0, 1.0));
    } else {
        rec.mat_color = default_color;
    }
    return true;
}
