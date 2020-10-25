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

    void SpotLight::SampleIntensity(const RayHitRecord& record, vec3& w_i, Spectrum& intensity) const {
        w_i = normalize(m_Position - record.hit_point);
        f32 cos_theta = max(dot(m_Direction, w_i), 0.0f);

        // Outside outer cone
        if (cos_theta < m_OuterCos) {
            intensity = {0.0f, 0.0f, 0.0f};
            return;
        }

        // Inside inner cone
        if (cos_theta >= m_InnerCos) {
            intensity = m_Intensity/distance2(m_Position, record.hit_point);
            return;
        }
        
        // Between inside and outside cone - quadric attenuation
        f32 delta = (cos_theta - m_OuterCos) / (m_InnerCos - m_OuterCos);
        intensity = (m_Intensity*(delta*delta)*(delta*delta))/distance2(m_Position, record.hit_point);
    }
}