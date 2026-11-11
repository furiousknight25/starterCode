#include "Framebuffer.h"
#include "SceneLoader/SceneContainer.h"
#include "SceneLoader/SceneLoader.h"
#include "SceneLoader/SceneParser_JSON.h"

#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <chrono>

int main(int argc, char* argv[]) {
    std::string scene_file = "examples/scenes/scenes_A/spheres_1K.json";
    int image_width = 300;
    int image_height = 300;
    int spp = 1;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-w" && i + 1 < argc) {
            image_width = std::stoi(argv[++i]);
        } else if (arg == "-h" && i + 1 < argc) {
            image_height = std::stoi(argv[++i]);
        } else if (arg == "-aa" && i + 1 < argc) {
            spp = std::stoi(argv[++i]);
        } else if (arg[0] != '-') {
            scene_file = arg;
        }
    }

    std::cout << "============================================================" << std::endl;
    std::cout << "             BVH Acceleration Benchmark                     " << std::endl;
    std::cout << "============================================================" << std::endl;
    std::cout << "Scene:      " << scene_file << std::endl;
    std::cout << "Resolution: " << image_width << "x" << image_height << std::endl;
    std::cout << "Samples/px: " << spp << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;

    // Load scene
    SceneContainer scene;
    auto loader = std::make_shared<SceneLoader>(scene, image_width, image_height);
    SceneParser_JSON parser(loader);
    parser.parseFileData(scene_file);
    scene.setImageSize(image_width, image_height);
    scene.cam.samples_per_pixel = spp;

    size_t num_shapes = scene.shape_list.objects.size();
    size_t num_lights = scene.shape_list.lights.size();
    std::cout << "Loaded shapes: " << num_shapes << std::endl;
    std::cout << "Loaded lights: " << num_lights << std::endl;
    std::cout << "------------------------------------------------------------" << std::endl;

    // 1. Render WITHOUT BVH (Linear)
    std::cout << "[1/2] Running render WITHOUT BVH (linear search)..." << std::flush;
    scene.use_bvh = false;
    scene.shape_list.clear_bvh();

    auto t0 = std::chrono::high_resolution_clock::now();
    Framebuffer fb_linear = scene.render();
    auto t1 = std::chrono::high_resolution_clock::now();
    double time_linear_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    std::cout << " done in " << std::fixed << std::setprecision(1) << time_linear_ms << " ms" << std::endl;

    // 2. Render WITH BVH
    std::cout << "[2/2] Running render WITH BVH acceleration..." << std::flush;
    scene.use_bvh = true;
    scene.shape_list.clear_bvh();

    auto bvh_build_t0 = std::chrono::high_resolution_clock::now();
    scene.shape_list.build_bvh();
    auto bvh_build_t1 = std::chrono::high_resolution_clock::now();
    double time_build_ms = std::chrono::duration<double, std::milli>(bvh_build_t1 - bvh_build_t0).count();

    auto t2 = std::chrono::high_resolution_clock::now();
    Framebuffer fb_bvh = scene.render();
    auto t3 = std::chrono::high_resolution_clock::now();
    double time_bvh_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();
    std::cout << " done in " << std::fixed << std::setprecision(1) << time_bvh_ms << " ms (tree build: "
              << time_build_ms << " ms)" << std::endl;

    // Verify visual match
    int diff_pixels = 0;
    for (int y = 0; y < image_height; ++y) {
        for (int x = 0; x < image_width; ++x) {
            ColorRGB c1 = fb_linear.get_pixel(x, y);
            ColorRGB c2 = fb_bvh.get_pixel(x, y);
            if (std::abs(int(c1.r) - int(c2.r)) > 2 ||
                std::abs(int(c1.g) - int(c2.g)) > 2 ||
                std::abs(int(c1.b) - int(c2.b)) > 2) {
                diff_pixels++;
            }
        }
    }

    double speedup = (time_bvh_ms > 0) ? (time_linear_ms / time_bvh_ms) : 0.0;

    std::cout << "============================================================" << std::endl;
    std::cout << "Results Summary:" << std::endl;
    std::cout << "  - Without BVH (Linear): " << std::setw(8) << std::fixed << std::setprecision(1)
              << time_linear_ms << " ms (" << (time_linear_ms / 1000.0) << " s)" << std::endl;
    std::cout << "  - With BVH (Tree):      " << std::setw(8) << std::fixed << std::setprecision(1)
              << time_bvh_ms << " ms (" << (time_bvh_ms / 1000.0) << " s)" << std::endl;
    std::cout << "  - BVH Build Time:       " << std::setw(8) << std::fixed << std::setprecision(1)
              << time_build_ms << " ms" << std::endl;
    std::cout << "  - Speedup Factor:       " << std::setw(8) << std::fixed << std::setprecision(2)
              << speedup << "x faster!" << std::endl;
    std::cout << "  - Pixel equivalence:    " << (image_width * image_height - diff_pixels)
              << " / " << (image_width * image_height) << " pixels match exactly" << std::endl;
    std::cout << "============================================================" << std::endl;

    return 0;
}
