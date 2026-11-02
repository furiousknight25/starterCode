#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
  public:
    ray() {}

    ray(const vec3& origin, const vec3& direction) : orig(origin), dir(direction) {}
    ray(const vec3& origin, const vec3& direction, const int& bounces) : orig(origin), dir(direction), boun(bounces)  {}

    // Returns a const reference to avoid copying while preventing modification
    const vec3& origin() const { return orig; }
    const vec3& direction() const { return dir; }
    const int bounces() const { return boun; }
    
    ray spawn_bounce(const vec3& new_origin, const vec3& new_direction) const {
      return ray(new_origin, new_direction, boun - 1);
    }


    vec3 at(double t) const { //return the point based on how far you are with t
        return orig + t*dir;
    }

  private:
    vec3 orig;
    vec3 dir;
    int boun = 5;
};

#endif
