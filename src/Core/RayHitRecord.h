#ifndef HELIOS_RAY_HIT_RECORD_H
#define HELIOS_RAY_HIT_RECORD_H

#include <memory>

#include <embree3/rtcore.h>
#include <glm/glm.hpp>

#include "Types.h"

namespace Helios {

    class BSDF;

    struct RayHitRecord{
        std::shared_ptr<BSDF> bsdf = nullptr;
        glm::vec3 hit_point;
        glm::vec3 normal; // normalized geometric normal
        glm::vec3 shading_normal;
        glm::vec3 tangent;
        glm::vec3 bitangent;
        glm::vec2 uv;
        f32 distance;
        u32 geometry_id;
        u32 primitive_id;
    };
}

#endif /* End of HELIOS_RAY_HIT_RECORD_H */