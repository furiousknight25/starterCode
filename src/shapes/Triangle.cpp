#include "Triangle.h"
#include <cmath>
#include <limits>

bool Triangle::intersect(const ray& r) const {
    hit_record rec;
    return intersect(r, 0.001, std::numeric_limits<double>::infinity(), rec);
}

bool Triangle::intersect(const ray& r, double t_min, double t_max, hit_record& rec) const {
    // Möller–Trumbore ray-triangle intersection algorithm
    const double EPS = 1e-12;
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;

    // Check if ray is parallel to triangle plane
    vec3 h = cross(r.direction(), edge2);
    double a = dot(edge1, h);

    if (std::abs(a) < EPS) {
        return false; // Ray is parallel to the triangle plane
    }

    // Calculate barycentric coordinate u
    double inv_a = 1.0 / a;
    vec3 s = r.origin() - v0;
    double u = inv_a * dot(s, h);

    if (u < 0.0 || u > 1.0) {
        return false; // Intersection lies outside the triangle
    }

    // Calculate barycentric coordinate v
    vec3 q = cross(s, edge1);
    double v = inv_a * dot(r.direction(), q);

    if (v < 0.0 || (u + v) > 1.0) {
        return false; // Intersection lies outside the triangle
    }

    // Calculate ray parameter t (distance to hit point)
    double t = inv_a * dot(edge2, q);

    if (t < t_min || t > t_max) {
        return false; // Hit is outside valid [t_min, t_max] range
    }

    rec.t = t;
    rec.p = r.at(t);
    vec3 outward_normal = unit_vector(cross(edge1, edge2));
    rec.set_face_normal(r, outward_normal);
    rec.mat_color = default_color;
    rec.is_sphere = false;
    rec.shader = shader;
    return true;
}
