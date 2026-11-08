#pragma once

#include "shape.h"
#include "shaders/Light.h"
#include <vector>
#include <memory>

//I only want a header, I will assume that there only ever will be one shapelist at a time if a shapelist does exist

class ShapeList : public Shape {
public:
    std::vector<std::shared_ptr<Shape>> objects;
    std::vector<Light> lights;

    ShapeList() = default;
    ShapeList(std::shared_ptr<Shape> object) { add(object); }

    void clear() { objects.clear(); }
    void add(std::shared_ptr<Shape> object) { objects.push_back(object); }
    void add_light(const Light& light) {lights.push_back(light);} // how do you have a const reference?

    bool intersect(const ray& r)const override { //basic multi, hard code buffer
        hit_record temp_rec;
        bool hit_anything = false;

        for (const auto& object : objects) {
            if (object ->intersect(r)) {
                hit_anything = true;
            }
        }
        
        return hit_anything;
    }

    bool intersect(const ray& r, double t_min, double t_max, hit_record& rec) const override {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest_so_far = t_max; //closest t value?

        for (const auto& object : objects) {
            if (object ->intersect(r, t_min, closest_so_far, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;

                rec = temp_rec;
            }
        }
        if (hit_anything) {
          rec.filtered_lights.clear();
          for (int i = 0; i < lights.size(); ++i) {
             
            ray blockr(temp_rec.p + (0.001f * temp_rec.normal), normalize(lights.at(i).position - temp_rec.p));
            
            bool in_shadow = false;
            for (const auto &obj : objects) {
                hit_record shadow_rec;
              in_shadow = obj->intersect(blockr, 0.001, (lights.at(i).position - temp_rec.p).length(), shadow_rec); //whats the point of shadow_rec?

              if (in_shadow == true) {
                  break;
              }
            }
            if (!in_shadow) {
                rec.filtered_lights.push_back(lights.at(i));
            }
          }
          rec.lights = &rec.filtered_lights;
        }

        return hit_anything;
    }//hit rec in the future have access to the shape list?
    bool intersect(const ray& r, interval ray_t, hit_record& rec) const override {
        return intersect(r, ray_t.min, ray_t.max, rec);
    }

};

using Scene = ShapeList;
using shape_list = ShapeList;
