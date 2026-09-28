#pragma once

#include "../math/vec3.h"
#include "../math/ray.h"

class Shape {
    public:
      vec3 position{0,0,0};
      vec3 rotation{0,0,0};
      
      Shape() = default; //what does default mean again?
      Shape(const vec3 &pos, const vec3 &rot = vec3(0, 0, 0)) : position(pos), rotation(rot) {}

      virtual ~Shape() = default; //how does a virtual deconstructor work?

      virtual bool intersect(const ray &r) const = 0;
      virtual bool intersect(const ray &r) { return const_cast<const Shape*>(this)->intersect(r); }
};