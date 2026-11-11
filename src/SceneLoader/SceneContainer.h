#pragma once

#include "../camera/PerspectiveCamera.h"
#include "../shapes/ShapeList.h"
#include "../Framebuffer.h"

struct SceneContainer {
    PerspectiveCamera cam;
    ShapeList shape_list;
    int image_width = 500;
    int image_height = 500;
    bool use_bvh = true;
    
    void setImageSize(int w, int h) {
        image_width = w;
        image_height = (h < 1) ? 1 : h;
        cam.image_width = image_width;
        cam.image_height = image_height;
        if (cam.image_plane_width > 0.0f) {
            cam.vp_width = cam.image_plane_width;
            cam.vp_height = cam.image_plane_width * (float(image_height) / float(image_width));
        } else {
            cam.vp_width = cam.vp_height * (float(image_width) / float(image_height));
        }
    }

    // Creates the framebuffer from camera dimensions, renders, and returns it
    Framebuffer render() {
        if (use_bvh && !shape_list.bvh_root && !shape_list.objects.empty()) {
            shape_list.build_bvh();
        } else if (!use_bvh) {
            shape_list.clear_bvh();
        }
        Framebuffer fb(cam.image_width, cam.image_height);
        cam.render(shape_list, fb);
        return fb;
    }
};
