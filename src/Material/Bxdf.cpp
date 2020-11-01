#include <glm/gtc/constants.hpp>

#include "Bxdf.h"

using namespace glm;

namespace Helios {
    Spectrum Lambertian::Evaluate(const vec3&, const vec3&) {
        return m_Reflectance*(one_over_pi<float>());
    }

    Spectrum SpecularBRDF::Evaluate(const vec3&, const vec3&) {
        return {0.0f, 0.0f, 0.0f};
    }

    Spectrum BSDF::Evaluate(const vec3& w_o, const vec3& w_i) const {
        Spectrum final_color = {0.0f, 0.0f, 0.0f};
        
        vec3 w_o_ts = world_to_tangent*w_o;
        vec3 w_i_ts = world_to_tangent*w_i;

        for (unsigned int i = 0; i < m_BxDFs.size(); i++) {
            final_color += m_BxDFs[i]->Evaluate(w_o_ts, w_i_ts);
        }

        return final_color;
    }
}