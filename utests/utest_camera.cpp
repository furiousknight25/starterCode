#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "camera/camera.h"
#include "camera/perspective_camera.h"

constexpr float EPSILON = 1.0e-5f;

TEST_CASE("Camera base dimensions and aspect ratio", "[camera]") {
    perspective_camera cam(400, 16.0f / 9.0f);

    REQUIRE(cam.image_width == 400);
    REQUIRE(cam.image_height == 225);
    REQUIRE_THAT(cam.aspect_ratio, Catch::Matchers::WithinAbs(16.0f / 9.0f, EPSILON));
}

TEST_CASE("Perspective camera ray generation", "[camera][perspective]") {
    perspective_camera cam(400, 16.0f / 9.0f, 1.0f, 2.0f, point3(0, 0, 0));

    SECTION("Origin of rays is camera center") {
        ray r = cam.generateRay(0, 0);
        REQUIRE_THAT(r.origin().x(), Catch::Matchers::WithinAbs(0.0f, EPSILON));
        REQUIRE_THAT(r.origin().y(), Catch::Matchers::WithinAbs(0.0f, EPSILON));
        REQUIRE_THAT(r.origin().z(), Catch::Matchers::WithinAbs(0.0f, EPSILON));
    }

    SECTION("Top-left pixel ray direction points toward -z and upper-left quadrant") {
        ray r00 = cam.generateRay(0, 0);
        // In the standard camera model, top-left pixel has negative x, positive y, and z = -focal_length (-1.0)
        CHECK(r00.direction().x() < 0.0);
        CHECK(r00.direction().y() > 0.0);
        REQUIRE_THAT(r00.direction().z(), Catch::Matchers::WithinAbs(-1.0f, EPSILON));
    }

    SECTION("Polymorphic ray generation via base camera pointer") {
        camera* base_cam = &cam;
        ray r = base_cam->generateRay(200, 112);
        REQUIRE_THAT(r.origin().z(), Catch::Matchers::WithinAbs(0.0f, EPSILON));
        REQUIRE_THAT(r.direction().z(), Catch::Matchers::WithinAbs(-1.0f, EPSILON));
    }
}

TEST_CASE("Perspective camera position functionality", "[camera][position]") {
    SECTION("Construct with non-zero position") {
        point3 initial_pos(3.0, -4.0, 5.0);
        perspective_camera cam(400, 16.0f / 9.0f, 1.0f, 2.0f, initial_pos);

        REQUIRE_THAT(cam.camera_center.x(), Catch::Matchers::WithinAbs(3.0f, EPSILON));
        REQUIRE_THAT(cam.camera_center.y(), Catch::Matchers::WithinAbs(-4.0f, EPSILON));
        REQUIRE_THAT(cam.camera_center.z(), Catch::Matchers::WithinAbs(5.0f, EPSILON));

        // Rays must originate from the camera center
        ray r0 = cam.generateRay(0, 0);
        REQUIRE_THAT(r0.origin().x(), Catch::Matchers::WithinAbs(3.0f, EPSILON));
        REQUIRE_THAT(r0.origin().y(), Catch::Matchers::WithinAbs(-4.0f, EPSILON));
        REQUIRE_THAT(r0.origin().z(), Catch::Matchers::WithinAbs(5.0f, EPSILON));

        ray r_center = cam.generateRay(200, 112);
        REQUIRE_THAT(r_center.origin().x(), Catch::Matchers::WithinAbs(3.0f, EPSILON));
        REQUIRE_THAT(r_center.origin().y(), Catch::Matchers::WithinAbs(-4.0f, EPSILON));
        REQUIRE_THAT(r_center.origin().z(), Catch::Matchers::WithinAbs(5.0f, EPSILON));
    }

    SECTION("Update position via set_position") {
        perspective_camera cam(400, 16.0f / 9.0f);
        cam.set_position(point3(10.0, 20.0, -30.0));

        REQUIRE_THAT(cam.camera_center.x(), Catch::Matchers::WithinAbs(10.0f, EPSILON));
        REQUIRE_THAT(cam.camera_center.y(), Catch::Matchers::WithinAbs(20.0f, EPSILON));
        REQUIRE_THAT(cam.camera_center.z(), Catch::Matchers::WithinAbs(-30.0f, EPSILON));

        ray r = cam.generateRay(150, 80);
        REQUIRE_THAT(r.origin().x(), Catch::Matchers::WithinAbs(10.0f, EPSILON));
        REQUIRE_THAT(r.origin().y(), Catch::Matchers::WithinAbs(20.0f, EPSILON));
        REQUIRE_THAT(r.origin().z(), Catch::Matchers::WithinAbs(-30.0f, EPSILON));
    }

    SECTION("Position reference alias is synchronized with camera_center") {
        perspective_camera cam(400, 16.0f / 9.0f);
        cam.position = point3(1.5, 2.5, 3.5);

        REQUIRE_THAT(cam.camera_center.x(), Catch::Matchers::WithinAbs(1.5f, EPSILON));
        REQUIRE_THAT(cam.camera_center.y(), Catch::Matchers::WithinAbs(2.5f, EPSILON));
        REQUIRE_THAT(cam.camera_center.z(), Catch::Matchers::WithinAbs(3.5f, EPSILON));

        ray r = cam.generateRay(0, 0);
        REQUIRE_THAT(r.origin().x(), Catch::Matchers::WithinAbs(1.5f, EPSILON));
        REQUIRE_THAT(r.origin().y(), Catch::Matchers::WithinAbs(2.5f, EPSILON));
        REQUIRE_THAT(r.origin().z(), Catch::Matchers::WithinAbs(3.5f, EPSILON));
    }
}

TEST_CASE("Perspective camera rotation functionality", "[camera][rotation]") {
    SECTION("Pitch rotation: 90 degrees up directs rays along +y") {
        perspective_camera cam(400, 16.0f / 9.0f);
        // Pitch 90 degrees tilts camera up towards +y
        cam.set_rotation(90.0f, 0.0f, 0.0f);

        // Center pixel (200, 112) should point predominantly along +y
        ray r_center = cam.generateRay(200, 112);
        vec3 dir = unit_vector(r_center.direction());
        CHECK(dir.y() > 0.99);
        REQUIRE_THAT(dir.z(), Catch::Matchers::WithinAbs(0.0f, 0.05f));
    }

    SECTION("Yaw rotation: 90 degrees right directs rays along +x") {
        perspective_camera cam(400, 16.0f / 9.0f);
        // Yaw 90 degrees turns camera right towards +x
        cam.set_rotation(0.0f, 90.0f, 0.0f);

        ray r_center = cam.generateRay(200, 112);
        vec3 dir = unit_vector(r_center.direction());
        CHECK(dir.x() > 0.99);
        REQUIRE_THAT(dir.z(), Catch::Matchers::WithinAbs(0.0f, 0.05f));
    }

    SECTION("Roll rotation: 90 degrees rotates viewport axes") {
        perspective_camera cam(400, 16.0f / 9.0f);
        cam.set_rotation(0.0f, 0.0f, 90.0f);

        // Right vector u should now point up (+y)
        REQUIRE_THAT(cam.u.x(), Catch::Matchers::WithinAbs(0.0f, EPSILON));
        REQUIRE_THAT(cam.u.y(), Catch::Matchers::WithinAbs(1.0f, EPSILON));
        REQUIRE_THAT(cam.u.z(), Catch::Matchers::WithinAbs(0.0f, EPSILON));

        // Up vector v should now point left (-x)
        REQUIRE_THAT(cam.v.x(), Catch::Matchers::WithinAbs(-1.0f, EPSILON));
        REQUIRE_THAT(cam.v.y(), Catch::Matchers::WithinAbs(0.0f, EPSILON));
        REQUIRE_THAT(cam.v.z(), Catch::Matchers::WithinAbs(0.0f, EPSILON));
    }

    SECTION("Orthonormal basis properties under arbitrary rotation") {
        perspective_camera cam(400, 16.0f / 9.0f);
        cam.set_rotation(25.0f, -40.0f, 15.0f);

        // Basis vectors must be unit length
        REQUIRE_THAT(cam.u.length(), Catch::Matchers::WithinAbs(1.0f, EPSILON));
        REQUIRE_THAT(cam.v.length(), Catch::Matchers::WithinAbs(1.0f, EPSILON));
        REQUIRE_THAT(cam.w.length(), Catch::Matchers::WithinAbs(1.0f, EPSILON));

        // Basis vectors must be mutually orthogonal
        REQUIRE_THAT(dot(cam.u, cam.v), Catch::Matchers::WithinAbs(0.0f, EPSILON));
        REQUIRE_THAT(dot(cam.v, cam.w), Catch::Matchers::WithinAbs(0.0f, EPSILON));
        REQUIRE_THAT(dot(cam.w, cam.u), Catch::Matchers::WithinAbs(0.0f, EPSILON));

        // Right-handed: u x v = w
        vec3 cross_uv = cross(cam.u, cam.v);
        REQUIRE_THAT(cross_uv.x(), Catch::Matchers::WithinAbs(cam.w.x(), EPSILON));
        REQUIRE_THAT(cross_uv.y(), Catch::Matchers::WithinAbs(cam.w.y(), EPSILON));
        REQUIRE_THAT(cross_uv.z(), Catch::Matchers::WithinAbs(cam.w.z(), EPSILON));
    }

    SECTION("Orient camera with set_lookat") {
        perspective_camera cam(400, 16.0f / 9.0f);
        cam.set_position(point3(0, 0, 0));
        // Pointing camera towards (0, 0, 10) (looking backward along +z)
        cam.set_lookat(point3(0, 0, 10));

        ray r_center = cam.generateRay(200, 112);
        vec3 dir = unit_vector(r_center.direction());
        CHECK(dir.z() > 0.99);
    }
}
