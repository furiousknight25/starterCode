#include "Framebuffer.h"
#include "SceneLoader/SceneContainer.h"
#include "SceneLoader/SceneLoader.h"
#include "SceneLoader/SceneParser_JSON.h"

#include <iostream>
#include <memory>
#include <string>

int main(int argc, char* argv[]) {
    std::string scene_file = "examples/scenes/scenes_A/oneSphere.json";
    std::string output_file = "json_output.png";
    int image_width = 500;
    int image_height = 500;
    int spp = 1;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-w" && i + 1 < argc) {
            image_width = std::stoi(argv[++i]);
        } else if (arg == "-h" && i + 1 < argc) {
            image_height = std::stoi(argv[++i]);
        } else if (arg == "-aa") {
            // Can be used either as "-aa" (default 16 samples) or "-aa <samples>"
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                spp = std::stoi(argv[++i]);
            } else {
                spp = 16;
            }
        } else if (arg == "-o" && i + 1 < argc) {
            output_file = argv[++i];
        } else if (arg[0] != '-') {
            scene_file = arg;
        }
    }

    std::cout << "Rendering scene: " << scene_file << std::endl;
    std::cout << "Resolution: -w " << image_width << " -h " << image_height;
    if (spp > 1) {
        std::cout << " (Anti-Aliasing: " << spp << " spp)";
    }
    std::cout << std::endl;

    SceneContainer scene;
    auto loader = std::make_shared<SceneLoader>(scene, image_width, image_height);
    SceneParser_JSON parser(loader);

    parser.parseFileData(scene_file);

    scene.setImageSize(image_width, image_height);
    scene.cam.samples_per_pixel = spp;

    Framebuffer fb = scene.render();
    fb.write_png(output_file);
    std::cout << "Render saved to: " << output_file << " (" << fb.width() << "x" << fb.height() << ")" << std::endl;

    return 0;
}
