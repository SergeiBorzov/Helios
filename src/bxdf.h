#ifndef HELIOS_BXDF_H
#define HELIOS_BXDF_H

#include <glm/glm.hpp>

#include "spectrum.h"

namespace Helios {
    class BxDF {
    public:
        virtual Spectrum Evaluate(const glm::vec3& w_o, const glm::vec3& w_i) = 0;
        virtual ~BxDF() {}
    };

    class Lambertian: public BxDF {
    public:
        Lambertian(const Spectrum& reflectance): m_Reflectance(reflectance) {}
        Spectrum Evaluate(const glm::vec3& w_o, const glm::vec3& w_i) override;
    private:
        Spectrum m_Reflectance;
    };
}

#endif /* End of HELIOS_BXDF_H */