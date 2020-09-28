#include <cstdio>

#include "globals.h"
#include "renderer.h"

namespace Helios {
    static Spectrum ray_tracing(const Scene& scene, const RTCRay& ray) {
        // hit_record = scene.Intersect(ray);
        RTCIntersectContext context;
        rtcInitIntersectContext(&context);

        RTCRayHit rayhit;
        rayhit.ray = ray;
        rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
        rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

        rtcIntersect1(scene.GetRTCScene(), &context, &rayhit);

        if (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
            return { 0.0f, 1.0f, 0.0f };
        }

        return { 0.0f, 0.0f, 0.0f };
    }

    void Renderer::Draw(std::vector<Spectrum>& buffer, const Scene& scene, int width, int height) {
        buffer.resize(width*height);

        // Render image from each camera of the scene
        for (const auto& camera: scene.m_Cameras) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    float u = 2.0f*(static_cast<float>(x) / width) - 1.0f;
                    float v = 2.0f*((height - y)/ static_cast<float>(height)) - 1.0f;
                    RTCRay ray = camera.GenerateRay(u, v);

                    buffer[y*width + x] = ray_tracing(scene, ray);
                }
            }
        }
    }
}