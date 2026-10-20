#include "Framebuffer.h"
#include "camera/PerspectiveCamera.h"
#include "math/ray.h"
#include "math/vec3.h"

#include <iostream>

#ifdef DEBUGVIS_ENABLED
  #include "DebugFramebuffer.h"
  using FB = DebugFramebuffer;
#else
  using FB = Framebuffer;
#endif

  color ray_color(const ray &r)
  {// color is 0.0-1.0
    vec3 dir = normalize(r.direction());

    dir = (dir + vec3(1.0, 1.0, 1.0)) * .5;// mapped to color space? ALSO dont redeclare things, so no need to "type cast it"
    return dir;
  }

int main() {
    FB fb(100,100);
    PerspectiveCamera cam(100, 100, 1.1);

    for (int y = 0; y < fb.height(); y++) {

      for (int x = 0; x < fb.width(); x++) {
        ray r = cam.shoot_ray(x,y);
        fb.set_pixel(x,y, ray_color(r));
      }
    }
    
    std::cout << cam.focal_length;
    fb.write_png("vectestnewworld.png");
}

//trying this?
// cmake --build buildVCPkg --target vectest
//  ./buildVCPkg/examples/vectest 