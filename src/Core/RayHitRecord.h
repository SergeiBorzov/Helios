#ifndef HELIOS_RAY_HIT_RECORD_H
#define HELIOS_RAY_HIT_RECORD_H

#include <memory>

#include <embree3/rtcore.h>
#include <glm/glm.hpp>

namespace Helios {

    class BSDF;

    struct RayHitRecord{
        glm::vec3 hit_point;
        glm::vec3 normal; // normalized geometric normal
        glm::vec3 shading_normal;
        std::shared_ptr<BSDF> bsdf = nullptr;
        float distance;
        unsigned int geometry_id;
        unsigned int primitive_id;
    };
}

#endif /* End of HELIOS_RAY_HIT_RECORD_H */