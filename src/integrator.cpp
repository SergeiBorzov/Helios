#include <cstdio>

#include "globals.h"
#include "integrator.h"

namespace Helios {
    Spectrum Integrator::RayTracing(const Scene& scene, const RTCRay& ray) {
        Spectrum final_color = { 0.0f, 0.0f, 0.0f };

        RayHitRecord hit_record;

        if (scene.Intersect(ray, hit_record)) {
            final_color = { 0.0f, 1.0f, 0.0f };
        }
        
        return final_color;
    }

    void Integrator::Render(std::vector<Spectrum>& buffer, const Scene& scene, int width, int height) {
        buffer.resize(width*height);

        // Render image from each camera of the scene
        for (const auto& camera: scene.m_Cameras) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    float u = 2.0f*(static_cast<float>(x) / width) - 1.0f;
                    float v = 2.0f*((height - y)/ static_cast<float>(height)) - 1.0f;
                    RTCRay ray = camera.GenerateRay(u, v);

                    buffer[y*width + x] = RayTracing(scene, ray);
                }
            }
        }
    }
}