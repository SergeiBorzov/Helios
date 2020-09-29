#ifndef HELIOS_BXDF_H
#define HELIOS_BXDF_H

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "../Core/Spectrum.h"

namespace Helios {
    class BxDF {
    public:
        // Note:: w_o and w_i are in tangent space
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

    class BSDF {
    public:
        // Note: w_o and w_i are in world space
        Spectrum Evaluate(const glm::vec3& w_o, const glm::vec3& w_i) const;
        void Add(std::unique_ptr<BxDF>&& bxdf) { m_BxDFs.push_back(std::move(bxdf)); }
    private:
        glm::mat3 world_to_tangent;
        glm::mat3 tangent_to_world;

        glm::vec3 normal;

        std::vector<std::unique_ptr<BxDF>> m_BxDFs;
    };
}

#endif /* End of HELIOS_BXDF_H */