#pragma once

#include "../camera/PerspectiveCamera.h"
#include "../shapes/ShapeList.h"
#include "../Framebuffer.h"

struct SceneContainer {
    PerspectiveCamera cam;
    ShapeList shape_list;
    int image_width = 500;
    int image_height = 500;
    
    void setImageSize(int w, int h) {
        image_width = w;
        image_height = (h < 1) ? 1 : h;
        cam.image_width = image_width;
        cam.image_height = image_height;
        cam.vp_width = cam.vp_height * (float(image_width) / float(image_height));
    }

    // Creates the framebuffer from camera dimensions, renders, and returns it
    Framebuffer render() {
        Framebuffer fb(cam.image_width, cam.image_height);
        cam.render(shape_list, fb);
        return fb;
    }
};
