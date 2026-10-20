#include "Framebuffer.h"
#include "camera/perspective_camera.h"
#include "math/ray.h"
#include "math/vec3.h"
#include "shapes/Sphere.h"

#include <iostream>
#include <string>

#ifdef DEBUGVIS_ENABLED
  #include "DebugFramebuffer.h"
  using FB = DebugFramebuffer;
#else
  using FB = Framebuffer;
#endif

int main() {
    FB fb(100,100);
    PerspectiveCamera cam(100,100,1.0);
    Sphere sphere(vec3(0,0,-10), 3.0f, color(.5,.5,.5));

    for (int y = 0; y < fb.height(); y++) {
      for (int x = 0; x < fb.width(); x++) {
        ray r = cam.shoot_ray(x,y);
        vec3 col = color(1.0f, 0.0f,0.0f);
        if (sphere.intersect(r)) {
            col = color(0.0f, 0.0f,1.0f);
        }
        
        fb.set_pixel(x,y, col);
      }
    }
    
    fb.write_png("sphere");
}
