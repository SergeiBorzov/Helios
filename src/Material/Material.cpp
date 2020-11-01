#include "Material.h"
#include "Bxdf.h"

using namespace glm;

namespace Helios {
    void Matte::ProduceBSDF(RayHitRecord& record) const {
        record.bsdf = std::make_shared<BSDF>(/* Record */);
        Spectrum hit_point_color = m_Diffuse;

        if (m_DiffuseTexture) {
            hit_point_color *= (*m_DiffuseTexture)(record.uv.x, record.uv.y);
        }

        if (m_NormalMap) {
            // Note: shading_normal and tangent are already normalized
            record.bitangent = cross(record.shading_normal, record.tangent);

            record.bsdf->tangent_to_world = mat3(record.tangent, record.bitangent, record.shading_normal);
            record.bsdf->world_to_tangent = transpose(record.bsdf->world_to_tangent);
            
            Spectrum n = (*m_NormalMap)(record.uv.x, record.uv.y);
            record.shading_normal = normalize(record.bsdf->tangent_to_world*normalize((2.0f*vec3(n.r, n.g, n.b) - 1.0f)));
        }
        record.bsdf->Add(std::make_unique<Lambertian>(hit_point_color));
    }

    void Mirror::ProduceBSDF(RayHitRecord& record) const {
        record.bsdf = std::make_shared<BSDF>(/* Record */);
        Spectrum hit_point_color = m_Specular;

        if (m_SpecularTexture) {
            hit_point_color *= (*m_SpecularTexture)(record.uv.x, record.uv.y);
        }

        if (m_NormalMap) {
            // Note: shading_normal and tangent are already normalized
            record.bitangent = cross(record.shading_normal, record.tangent);

            record.bsdf->tangent_to_world = mat3(record.tangent, record.bitangent, record.shading_normal);
            record.bsdf->world_to_tangent = transpose(record.bsdf->world_to_tangent);
            
            Spectrum n = (*m_NormalMap)(record.uv.x, record.uv.y);
            record.shading_normal = normalize(record.bsdf->tangent_to_world*normalize((2.0f*vec3(n.r, n.g, n.b) - 1.0f)));
        }
        record.bsdf->Add(std::make_unique<Lambertian>(hit_point_color));
    }
}