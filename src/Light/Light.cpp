#include <glm/gtx/norm.hpp>

#include "Light.h"

using namespace glm;

namespace Helios {
    void DirectionalLight::SampleIntensity(const RayHitRecord& record, vec3& w_i, Spectrum& intensity) const {
        w_i = m_Direction;
        intensity = m_Intensity;
    }

    void PointLight::SampleIntensity(const RayHitRecord& record, vec3& w_i, Spectrum& intensity) const {
        w_i = normalize(m_Position - record.hit_point);
        intensity = m_Intensity/distance2(m_Position, record.hit_point);
    }
}