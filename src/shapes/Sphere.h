#pragma once

#include "shape.h"


class Sphere : public Shape {
public:
    float radius = 1.0;

    Sphere() = default;
    Sphere(const vec3 &center, float r, const color& col = color(0.8, 0.2, 0.2))
        : Shape(center, vec3(0, 0, 0), col), radius(r) {}
    Sphere(const vec3 &center, float r, std::shared_ptr<Shader> s)
        : Shape(center, vec3(0, 0, 0), color(0.8, 0.2, 0.2), s), radius(r) {}
    Sphere(const vec3 &center, float r, const color& col, std::shared_ptr<Shader> s)
        : Shape(center, vec3(0, 0, 0), col, s), radius(r) {}

    bool intersect(const ray& r) const {
      // ax^2 + bx + c = 0
      float a = dot(r.direction(), r.direction());
      float b = dot((-2 * r.direction()), (position - r.origin()));
      float c = dot((position - r.origin()), position - r.origin()) - pow(radius, 2);

      // quadratic equation verification
      float discrim = b * b - 4 * a * c;// all ya need is this :)
      return (discrim >= 0);
    }

    bool intersect(const ray &r, double t_min, double t_max, hit_record &rec) const
    { 
      // optimized math
    
      vec3 oc = position - r.origin();
    auto a = r.direction().length_squared();
    auto h = dot(r.direction(), oc);
    auto c = oc.length_squared() - radius*radius;
    auto discrim = h * h - a * c;
    
    if (discrim < 0) return false; //not possible

    auto sqrtd = std::sqrt(discrim);

    auto root = (h - sqrtd) / a;// finishing the quadratic formula big equation

    if (root <= t_min || t_max <= root) {
      root = (h + sqrtd) / a;// check for the + in the -+
      if (root <= t_min || t_max <= root) { return false; }
      // check both the - and the positive root
    }
      
      //ITS REAL, PREPARE THE DATAAA
      rec.t = root;
      rec.p = r.at(rec.t);
      vec3 outward_normal = (rec.p - position) / radius; 
      //easy math also to tell what the outside of a zamn sphere is
      //this math is juts faster than normalize which does sqrt which is slow
      
      rec.view_dir = normalize(-r.direction());
      rec.set_face_normal(r, outward_normal);
      //dictates what the real normal is depending on the angle of the ray
      rec.shader = shader; //attach this too :)
      
      return true;
    }
};
