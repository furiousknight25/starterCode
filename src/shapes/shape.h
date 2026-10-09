#pragma once

#include "../math/vec3.h"
#include "../math/ray.h"
#include "../math/interval.h"
#include "hit_record.h"

class Shape {
    public:
      vec3 position{0,0,0};
      vec3 rotation{0,0,0};
      color default_color{0.8, 0.8, 0.8};
      std::shared_ptr<Shader> shader{nullptr}; // all shapes have a shader!
      
      Shape() = default; //what does default mean again?
      Shape(const vec3 &pos, const vec3 &rot = vec3(0, 0, 0), const color& col = color(0.8, 0.8, 0.8), std::shared_ptr<Shader> sh = nullptr)
          : position(pos), rotation(rot), default_color(col), shader(sh) {}

      virtual ~Shape() = default; //how does a virtual deconstructor work?

      void set_shader(std::shared_ptr<Shader> s) { shader = s; }
      std::shared_ptr<Shader> get_shader() const { return shader; }

      // Basic boolean intersection (from lab 1)
      virtual bool intersect(const ray &r) const = 0;
      virtual bool intersect(const ray &r) { return const_cast<const Shape*>(this)->intersect(r); }

      // Range-bounded intersection with hit_record (for occlusion and shading)
      virtual bool intersect(const ray &r, double t_min, double t_max, hit_record &rec) const {
          return false;
      }
      virtual bool intersect(const ray &r, interval ray_t, hit_record &rec) const {
          return intersect(r, ray_t.min, ray_t.max, rec);
      }
};