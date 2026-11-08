#pragma once

#include "../math/vec3.h"
#include "../shaders/Light.h"
#include "../math/ray.h"
#include <memory>
#include <vector>

class Shader; // forward declaration so we can store a pointer to the shader

struct hit_record {
    vec3 p{0.0, 0.0, 0.0}; //position
    vec3 normal{0.0, 0.0, 1.0};
    float t{0.0}; //I want floatssss
                  //this is the "same" as float t = 0.0;
    bool front_face{true};
    
    color mat_color{0.8, 0.8, 0.8};
    bool is_sphere{false};

    // View direction points back towards the camera/eye from hit point
    vec3 view_dir{0.0, 0.0, 1.0};

    // Pointer to shader of the shape we hit
    std::shared_ptr<Shader> shader{nullptr};//why not just have a reference to the shape 

    std::vector<Light> filtered_lights;
    const std::vector<Light>* lights = nullptr;

    //A: pasing the vec3& without the & counts as a copy not a value
    //note in the case of a sphere the outward normal is easily calcualted since its just sphere_center - radius or hit_pos
    void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;        
        //are outward normal and ray facing same direction?

        normal = front_face ? outward_normal : -outward_normal;
        //if cam on the inside, normal is flipped to be facing inside
        //if outside normal flipped on the outside

        //lowk I kinda dont like this I like the set normals that things like blender has  ithink
        //well thats a future problem

    }
};