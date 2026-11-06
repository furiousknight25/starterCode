#pragma once

#include "../math/ray.h"
#include "../math/vec3.h"
#include "../shapes/shape.h"
#include "../shapes/ShapeList.h"
#include "../shaders/Shader.h"
#include "Framebuffer.h"
#include <random>

// Minimal abstract base class for cameras
class Camera {
public:
    vec3 position = vec3(0, 0, 0);
    vec3 rotation = vec3(0, 0, 0);

    float vp_width = 2.0f;
    float vp_height = 2.0f;

    int image_width = 100; //alright I GUESS it gets defined here, framebuffer pulls, kinda weird though lol
    int image_height = 100;
    int samples_per_pixel = 1; // Anti-aliasing samples per pixel

    Camera() = default;

    Camera(int width, int height, float focal_length = 1.0f, float v_height = 2.0f,
           vec3 pos = vec3(0, 0, 0), vec3 rot = vec3(0, 0, 0))
        : position(pos), rotation(rot),
          vp_height(v_height),
          vp_width(v_height * (float(width) / float(height < 1 ? 1 : height))),
          image_width(width), image_height(height < 1 ? 1 : height) {}

    virtual ~Camera() = default;

    virtual ray shoot_ray(float x, float y) const = 0;
    
    void render(const ShapeList& scene, Framebuffer& fb) const {
        for (int y = 0; y < image_height; ++y) {
            for (int x = 0; x < image_width; ++x) {
                color pixel_color(0.0f, 0.0f, 0.0f);

                for (int s = 0; s < samples_per_pixel; ++s) {
                    // Jitter subpixel offset in [-0.5, 0.5] for anti-aliasing
                    float offset_x = (samples_per_pixel > 1) ? (random_float() - 0.5f) : 0.0f;
                    float offset_y = (samples_per_pixel > 1) ? (random_float() - 0.5f) : 0.0f;

                    ray r = shoot_ray(x + offset_x, y + offset_y);
                    pixel_color += ray_color(r, scene);
                }

                fb.set_pixel(x, y, pixel_color / float(samples_per_pixel));
            }
        }
    }
    
    color ray_color(const ray& r, const ShapeList& scene) const {
        if (r.bounces() <= 0) {
            return vec3(0.0f, 0.0f, 0.0f);
        }
        
        hit_record rec;
        if (scene.intersect(r, 0.001, 1000.0, rec)) {
            ray scattered;
            color attenuation; //what is attenuation again

            if (rec.shader && rec.shader->scatter(r, rec, attenuation, scattered)) {
                return attenuation * ray_color(scattered, scene);
            }

            if (rec.shader) {
                return rec.shader->shape_render(rec);
            }
            return color(1.0f,0.0f,1.0f);
        }

        return color(0.0f, 0.0f, 0.0f); //black background
    }

private:
    static float random_float() {
        static thread_local std::mt19937 generator(1337);
        static thread_local std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        return distribution(generator);
    }
};
