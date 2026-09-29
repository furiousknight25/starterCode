#include "Triangle.h"
#include <cmath>
#include <limits>

bool Triangle::intersect(const ray& r) const {
    hit_record rec;
    return intersect(r, 0.001, std::numeric_limits<double>::infinity(), rec);
}

bool Triangle::intersect(const ray& r, double t_min, double t_max, hit_record& rec) const {
    const double EPS = 1e-12;
    vec3 edge1 = v1 - v0;
    vec3 edge2 = v2 - v0;

    vec3 h = cross(r.direction(), edge2);
    double a = dot(edge1, h);

    if (std::abs(a) < EPS) {
        return false; // Ray is parallel to the triangle plane
    }

    double inv_a = 1.0 / a;
    vec3 s = r.origin() - v0;
    double u = inv_a * dot(s, h);

    if (u < 0.0 || u > 1.0) {
        return false;
    }

    vec3 q = cross(s, edge1);
    double v = inv_a * dot(r.direction(), q);

    if (v < 0.0 || (u + v) > 1.0) {
        return false;
    }

    double t = inv_a * dot(edge2, q);

    if (t < t_min || t > t_max) {
        return false;
    }

    rec.t = t;
    rec.p = r.at(t);
    vec3 outward_normal = unit_vector(cross(edge1, edge2));
    rec.set_face_normal(r, outward_normal);
    rec.mat_color = default_color;
    rec.is_sphere = false;
    return true;
}
