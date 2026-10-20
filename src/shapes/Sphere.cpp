#include "Sphere.h"
#include <cmath>

bool Sphere::intersect(const ray& r) const { 
    //ax^2 + bx + c = 0
    float a = dot(r.direction(), r.direction());
    float b = dot((-2 * r.direction()), (position - r.origin()));
    float c = dot((position - r.origin()), position - r.origin()) - pow(radius,2);

    //quadratic equation verification
    float discrim = b*b - 4*a*c; //all ya need is this :)
    return (discrim >= 0);
}

bool Sphere::intersect(const ray& r, double t_min, double t_max, hit_record& rec) const {
    vec3 oc = r.origin() - position;
    double a = dot(r.direction(), r.direction());
    if (a <= 1e-12) {
        return false;
    }
    double b = 2.0 * dot(r.direction(), oc);
    double c = dot(oc, oc) - (radius * radius);
    double discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0) {
        return false;
    }

    double sqrt_d = std::sqrt(discriminant);

    // Find the nearest root that lies in the acceptable range [t_min, t_max]
    double root = (-b - sqrt_d) / (2.0 * a);
    if (root < t_min || root > t_max) {
        root = (-b + sqrt_d) / (2.0 * a);
        if (root < t_min || root > t_max) {
            return false;
        }
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outward_normal = normalize(rec.p - position);
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
