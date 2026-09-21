#include <catch2/catch_test_macros.hpp>
#include "Framebuffer.h"
#include "math/vec3.h"

TEST_CASE("Framebuffer initialization and dimensions", "[framebuffer]") {
    Framebuffer fb(800, 600);

    REQUIRE(fb.width() == 800);
    REQUIRE(fb.height() == 600);
}

TEST_CASE("Framebuffer pixel read and write", "[framebuffer]") {
    Framebuffer fb(10, 10);
    ColorRGB test_color{255, 128, 64};

    SECTION("Set and get pixel with ColorRGB") {
        fb.set_pixel(3, 4, test_color);
        ColorRGB result = fb.get_pixel(3, 4);

        REQUIRE(result.r == 255);
        REQUIRE(result.g == 128);
        REQUIRE(result.b == 64);
    }

    SECTION("Set pixel with vec3 converts normalized values to 8-bit bytes") {
        vec3 color_normalized(1.0, 0.5, 0.0);
        fb.set_pixel(2, 2, color_normalized);
        ColorRGB result = fb.get_pixel(2, 2);

        REQUIRE(result.r == 255);
        REQUIRE(result.g == 128);
        REQUIRE(result.b == 0);
    }
}

TEST_CASE("Framebuffer bounds safety", "[framebuffer]") {
    Framebuffer fb(5, 5);
    ColorRGB out_of_bounds_color{255, 255, 255};

    SECTION("Writes outside width and height do not crash") {
        fb.set_pixel(-1, 0, out_of_bounds_color);
        fb.set_pixel(0, -1, out_of_bounds_color);
        fb.set_pixel(5, 2, out_of_bounds_color);  // Valid x are 0-4
        fb.set_pixel(2, 5, out_of_bounds_color);  // Valid y are 0-4

        // Getting out-of-bounds coordinates should safely return default black
        ColorRGB fallback = fb.get_pixel(5, 5);
        REQUIRE(fallback.r == 0);
        REQUIRE(fallback.g == 0);
        REQUIRE(fallback.b == 0);
    }
}

TEST_CASE("Framebuffer clear operation", "[framebuffer]") {
    Framebuffer fb(4, 4);
    ColorRGB white{255, 255, 255};
    ColorRGB blue{0, 0, 255};

    fb.clear(white);
    REQUIRE(fb.get_pixel(0, 0).r == 255);
    REQUIRE(fb.get_pixel(3, 3).r == 255);

    fb.clear(blue);
    REQUIRE(fb.get_pixel(1, 1).b == 255);
    REQUIRE(fb.get_pixel(1, 1).r == 0);
}
