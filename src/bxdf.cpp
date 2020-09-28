#include <glm/gtc/constants.hpp>

#include "bxdf.h"

using glm::vec3;

namespace Helios {
    Spectrum Lambertian::Evaluate(const vec3&, const vec3&) {
        return m_Reflectance*(glm::one_over_pi<float>());
    }
}