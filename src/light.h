#ifndef HELIOS_LIGHT_H
#define HELIOS_LIGHT_H

#include <glm/glm.hpp>

#include "spectrum.h"

namespace Helios {

    class Light {
    public:
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

        ~DirectionalLight() {}
    private:
        glm::vec3 m_Direction;
    };

}
#endif /* End of Helios */