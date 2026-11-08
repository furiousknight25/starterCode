#pragma once

#include "ISceneLoader.h"
#include "SceneContainer.h"
#include "../shapes/Sphere.h"
#include "../shapes/Triangle.h"
#include "../shaders/LambertianShader.h"
#include "../shaders/BlinnPhong.h"
#include "../shaders/MirrorShader.h"
#include "../shaders/NormalShader.h"
#include "../shaders/SinShader.h"

#include <unordered_map>
#include <iostream>
#include <memory>

class SceneLoader : public ISceneLoader {
public:
    SceneContainer& m_scene;
    int image_width  = 500;  // Default render resolution
    int image_height = 500;

    // Lookup table: JSON shapes reference shaders by string name
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;

    SceneLoader(SceneContainer& scene, int width = 500, int height = 500)
        : m_scene(scene), image_width(width), image_height(height) {
        m_scene.setImageSize(width, height);
    }

    void setImageSize(int w, int h) {
        image_width = w;
        image_height = (h < 1) ? 1 : h;
        m_scene.setImageSize(image_width, image_height);
    }

    // Helper to convert ISceneLoader::vec to your vec3
    vec3 to_vec3(const ISceneLoader::vec& v) {
        return vec3(v.x, v.y, v.z);
    }

    void setSceneParams(const SceneParams& sparams) override {}

    // --- Camera ---
    void addCamera(const std::string& name, const std::string& type,
                   const ISceneLoader::vec& pos, const ISceneLoader::vec& viewDir,
                   float focalLength, float imagePlaneWidth) override {
        std::cout << "Loading camera: " << name << " [resolution: " << image_width << "x" << image_height << "]" << std::endl;
        m_scene.cam = PerspectiveCamera(image_width, image_height, focalLength);
        m_scene.cam.position = to_vec3(pos);
    }

    // --- Lights ---
    void addPointLight(const ISceneLoader::vec& pos,
                       const ISceneLoader::vec& intensity) override {
        std::cout << "Loading point light" << std::endl;
        Light light(to_vec3(pos), to_vec3(intensity));
        m_scene.shape_list.add_light(light);
    }

    // --- Shaders ---
    void addShader(const ISceneLoader::ShaderDesc& desc) override {
        std::cout << "Loading shader: " << desc.name
                  << " (type=" << desc.type << ")" << std::endl;

        std::shared_ptr<Shader> shader;

        if (desc.type == "Lambertian" || desc.type == "Diffuse") {
            vec3 diffuse = to_vec3(desc.diffuse.data);
            shader = std::make_shared<LambertianShader>(diffuse);
        }
        else if (desc.type == "BlinnPhong" || desc.type == "Phong") {
            vec3 diffuse = to_vec3(desc.diffuse.data);
            float phong_exp = desc.phongExp;
            shader = std::make_shared<BlinnPhong>(diffuse, phong_exp);
        }
        else if (desc.type == "Mirror") {
            shader = std::make_shared<MirrorShader>();
        }
        else if (desc.type == "SinShader" || desc.type == "sin_shader" || desc.type == "Sin_Shader" || desc.type == "sinshader" || desc.type == "Sin" || desc.type == "sin") {
            shader = std::make_shared<SinShader>();
        }
        else {
            std::cout << "  Unknown shader type '" << desc.type
                      << "', falling back to NormalShader" << std::endl;
            shader = std::make_shared<NormalShader>();
        }

        shaders[desc.name] = shader;
    }

    // --- Shapes ---
    void addShape(const ISceneLoader::ShapeDesc& desc) override {
        std::cout << "Loading shape: " << desc.name
                  << " (type=" << desc.type << ")" << std::endl;

        // Look up the shape's shader by name reference
        std::shared_ptr<Shader> shader = nullptr;
        if (!desc.shaderNameReference.empty() && shaders.count(desc.shaderNameReference)) {
            shader = shaders[desc.shaderNameReference];
        }

        if (desc.type == "sphere") {
            vec3 center = to_vec3(desc.center);
            float radius = desc.radius;
            m_scene.shape_list.add(
                std::make_shared<Sphere>(center, radius, shader)
            );
        }
        else if (desc.type == "triangle") {
            vec3 v0 = to_vec3(desc.v0);
            vec3 v1 = to_vec3(desc.v1);
            vec3 v2 = to_vec3(desc.v2);

            if (desc.hasXForm) {
                glm::vec4 p0 = desc.localXForm * glm::vec4(v0.x(), v0.y(), v0.z(), 1.0f);
                glm::vec4 p1 = desc.localXForm * glm::vec4(v1.x(), v1.y(), v1.z(), 1.0f);
                glm::vec4 p2 = desc.localXForm * glm::vec4(v2.x(), v2.y(), v2.z(), 1.0f);
                v0 = vec3(p0.x, p0.y, p0.z);
                v1 = vec3(p1.x, p1.y, p1.z);
                v2 = vec3(p2.x, p2.y, p2.z);
            }

            m_scene.shape_list.add(
                std::make_shared<Triangle>(v0, v1, v2, shader)
            );
        }
        else {
            std::cout << "  Shape type '" << desc.type << "' skipped (unsupported/out of scope)." << std::endl;
        }
    }
};
