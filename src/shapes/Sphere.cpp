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
