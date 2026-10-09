#include <catch2/catch_test_macros.hpp>
#include "Framebuffer.h"
#include "math/vec3.h"

TEST_CASE("Framebuffer initialization and dimensions", "[framebuffer]") {
    Framebuffer fb(800, 600);

    REQUIRE(fb.width() == 800);
    REQUIRE(fb.height() == 600);
}


TEST_CASE("weird size frame", "[framebuffer]") { //what is the "[framebuffer part]"
    Framebuffer fb(20, 2);

    fb.set_pixel(19,0, ColorRGB(55,55,55));
    fb.set_pixel(0,1, ColorRGB(1,1,1));
    
    REQUIRE(fb.get_pixel(19,0) == ColorRGB(55,55,55));
}
