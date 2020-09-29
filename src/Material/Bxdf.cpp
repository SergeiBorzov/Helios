#include <glm/gtc/constants.hpp>

#include "Bxdf.h"

using glm::vec3;

namespace Helios {
    Spectrum Lambertian::Evaluate(const vec3&, const vec3&) {
        return m_Reflectance*(glm::one_over_pi<float>());
    }

    Spectrum BSDF::Evaluate(const vec3& w_o, const vec3& w_i) const {
        Spectrum final_color = {0.0f, 0.0f, 0.0f};
        
        glm::vec3 w_o_ts = /*TBN*/w_o;
        glm::vec3 w_i_ts = /*TBN*/w_i;

        for (unsigned int i = 0; i < m_BxDFs.size(); i++) {
            final_color += m_BxDFs[i]->Evaluate(w_o_ts, w_i_ts);
        }

        return final_color;
    }
}