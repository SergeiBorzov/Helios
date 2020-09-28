#include <cstdio>

#include "globals.h"
#include "renderer.h"

namespace Helios {
    Spectrum Renderer::RayTracing(const Scene& scene, const RTCRay& ray) {
        // hit_record = scene.Intersect(ray);
        RTCIntersectContext context;
        rtcInitIntersectContext(&context);

        RTCRayHit rayhit;
        rayhit.ray = ray;
        rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
        rayhit.hit.instID[0] = RTC_INVALID_GEOMETRY_ID;

        rtcIntersect1(scene.m_Scene, &context, &rayhit);

        unsigned int geometry_id = rayhit.hit.geomID;

        //Spectrum final_color = {0.0f, 0.0f, 0.0f}
        if (geometry_id != RTC_INVALID_GEOMETRY_ID) {
            /*const Material* mat = scene.m_MaterialMap[geometry_id];

            for (unsigned int i = 0; i < scene.m_Lights[i]; i++) {
                // get w_i;
                mat->ProduceBSDF(//hit_record*);
                Spectrum color = hit_record.bsdf->Evaluate(w_i, w_o);

                if (//reach_light) {
                    final_color += light_intensity*color;
                }
            }*/

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

                    buffer[y*width + x] = RayTracing(scene, ray);
                }
            }
        }
    }
}