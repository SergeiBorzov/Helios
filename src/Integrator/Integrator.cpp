#include <cstdio>

#include "../Core/Globals.h"
#include "../Material/Bxdf.h"
#include "Integrator.h"

using glm::vec3;

namespace Helios {
    Spectrum Integrator::RayTracing(const Scene& scene, const RTCRay& ray) {
        Spectrum final_color = { 0.0f, 0.0f, 0.0f };
        vec3 w_o = -vec3(ray.dir_x, ray.dir_y, ray.dir_z);

        RayHitRecord hit_record;

        if (scene.Intersect(ray, hit_record)) {
            auto it = scene.m_MaterialMap.find(hit_record.geometry_id);
            assert(it != scene.m_MaterialMap.end());

            const Material* material = scene.m_Materials[it->second];
            material->ProduceBSDF(hit_record);

            for (unsigned int i = 0; i < scene.m_Lights.size(); i++) {
                Spectrum light_intensity = {0.0f, 0.0f, 0.0f};
                vec3 w_i = vec3(0.0f);
                scene.m_Lights[i]->SampleIntensity(w_i, light_intensity);

                Spectrum color = hit_record.bsdf->Evaluate(w_o, w_i);

                RTCRay shadow_ray;
                shadow_ray.org_x = hit_record.hit_point.x;
                shadow_ray.org_y = hit_record.hit_point.y;
                shadow_ray.org_z = hit_record.hit_point.z;
                shadow_ray.tnear = 0.0f;
                shadow_ray.dir_x = w_i.x;
                shadow_ray.dir_y = w_i.y;
                shadow_ray.dir_z = w_i.z;
                shadow_ray.tfar = std::numeric_limits<float>::infinity();
                shadow_ray.mask = -1;
                shadow_ray.flags = 0;

                if (!scene.Occluded(shadow_ray)) {
                    final_color += color*light_intensity*glm::max(glm::dot(w_i, hit_record.shading_normal), 0.0f);
                }
            }
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