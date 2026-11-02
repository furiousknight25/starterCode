#include "Framebuffer.h"
#include "camera/PerspectiveCamera.h"
#include "math/ray.h"
#include "math/vec3.h"
#include "shapes/Sphere.h"
#include "shapes/ShapeList.h"
#include "shaders/NormalShader.h"
#include "shaders/LambertianShader.h"
#include "shaders/Light.h"
#include "shaders/BlinnPhong.h"
#include "shaders/MirrorShader.h"

#include <iostream>
#include <memory>
#include <random>
#include <string>

#ifdef DEBUGVIS_ENABLED
  #include "DebugFramebuffer.h"
  using FB = DebugFramebuffer;
#else
  using FB = Framebuffer;
#endif

// Pseudo-random uniform real in [0, 1)
inline double random_double() {
    static thread_local std::mt19937 generator(1337);
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(generator);
}


int main() {
    
    FB fb(300,300);
    PerspectiveCamera cam(300,300,1.0f);
    
    ShapeList scene; //why is it not ShapeList scene(); default constructer this way right?

    Light light(vec3(0, -10, -5), vec3(1.0,1.0,1.0), 2.0f);
    scene.add_light(light);

    auto normal_shader = std::make_shared<NormalShader>();
    auto lambertian_shader = std::make_shared<LambertianShader>(vec3(0.0f,0.0f,1.0f));
    auto blinn_phong = std::make_shared<BlinnPhong>(vec3(0.0f,0.0f,1.0f), 150.0f);
    auto mirror_shader = std::make_shared<MirrorShader>();

    scene.add(std::make_shared<Sphere>(vec3(0,0,-10), 3.0f, normal_shader));
    scene.add(std::make_shared<Sphere>(vec3(7,3,-10), 2.0f, lambertian_shader));
    scene.add(std::make_shared<Sphere>(vec3(-7,0,-10), 2.0f, blinn_phong));
    scene.add(std::make_shared<Sphere>(vec3(-7,-4,-12), 2.0f, mirror_shader));
    scene.add(std::make_shared<Sphere>(vec3(-10,-4,-12), 2.0f, mirror_shader));
    scene.add(std::make_shared<Sphere>(vec3(-2,-4,-12), 2.0f, mirror_shader));
    scene.add(std::make_shared<Sphere>(vec3(6,-4,-12), 2.0f, mirror_shader));
    
    scene.add(std::make_shared<Sphere>(vec3(-7,5,-12), 2.0f, mirror_shader));
    scene.add(std::make_shared<Sphere>(vec3(-10,8,-12), 2.0f, mirror_shader));

    cam.render(scene, fb);
    
    fb.write_png("double_scene.png");
    

}


