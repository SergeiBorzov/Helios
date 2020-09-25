#ifndef HELIOS_LIGHT_H
#define HELIOS_LIGHT_H

#include <Eigen/Core>

#include "spectrum.h"

namespace Helios {
    class Light {
    public:
        enum Type {
            Directional
        };
    protected:
        Light(const Spectrum& intensity, Type type): 
            m_Intensity(intensity), m_Type(type) 
        {}

        Spectrum m_Intensity;
        Type m_Type;
    };

    class DirectionalLight: public Light {
    public:
        DirectionalLight(const Eigen::Vector3f& direction, const Spectrum& intensity): 
            Light(intensity, Type::Directional),
            m_Direction(direction)
        {}
    private:
        Eigen::Vector3f m_Direction;
    };

}
#endif /* End of Helios */