#include "Light.h"

using glm::vec3;

namespace Helios {
    void DirectionalLight::SampleIntensity(vec3& w_i, Spectrum& intensity) const {
        w_i = m_Direction;
        intensity = m_Intensity;
    }
}