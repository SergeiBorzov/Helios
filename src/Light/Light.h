#ifndef HELIOS_LIGHT_H
#define HELIOS_LIGHT_H

#include <glm/glm.hpp>

#include "../Core/Spectrum.h"

namespace Helios {

    class Light {
    public:
        virtual void SampleIntensity(glm::vec3& w_i, Spectrum& intensity) const = 0;
        virtual ~Light() {}
    protected:
        Light(const Spectrum& intensity): m_Intensity(intensity) {}
       
        Spectrum m_Intensity;
    };

    class DirectionalLight: public Light {
    public:
        DirectionalLight(const glm::vec3& direction, const Spectrum& intensity): 
            Light(intensity), m_Direction(direction)
        {}

        void SampleIntensity(glm::vec3& w_i, Spectrum& intensity) const override;

        ~DirectionalLight() {}
    private:
        glm::vec3 m_Direction;
    };

}
#endif /* End of Helios */