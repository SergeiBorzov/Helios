#ifndef HELIOS_LIGHT_H
#define HELIOS_LIGHT_H

#include <glm/glm.hpp>

#include "../Core/Spectrum.h"
#include "../Core/RayHitRecord.h"

namespace Helios {

    class Light {
    public:
        virtual void SampleIntensity(const RayHitRecord& record, glm::vec3& w_i, Spectrum& intensity) const = 0;
        virtual ~Light() {}
    protected:
        Light(const Spectrum& intensity): m_Intensity(intensity) {}
       
        Spectrum m_Intensity;
    };

    class DirectionalLight: public Light {
    public:
        DirectionalLight(const glm::vec3& direction, const Spectrum& intensity): 
            Light(intensity), 
            m_Direction(glm::normalize(direction))
        {}

        void SampleIntensity(const RayHitRecord& record, glm::vec3& w_i, Spectrum& intensity) const override;

        ~DirectionalLight() {}
    private:
        glm::vec3 m_Direction;
    };

    class PointLight: public Light {
    public:
        PointLight(const glm::vec3& position, const Spectrum& intensity):
            Light(intensity), 
            m_Position(position)
        {}

        void SampleIntensity(const RayHitRecord& record, glm::vec3& w_i, Spectrum& intensity) const override;
        ~PointLight() {}
    private:
        glm::vec3 m_Position;
    };

    class SpotLight: public Light {
    public:
        SpotLight(const glm::vec3& position, const glm::vec3& direction, f32 outer_angle, const Spectrum& intensity):
            Light(intensity),
            m_Position(position),
            m_Direction(glm::normalize(direction)),
            m_OuterCos(glm::cos(outer_angle))
        {}

        void SampleIntensity(const RayHitRecord& record, glm::vec3& w_i, Spectrum& intensity) const override;
    private:
        glm::vec3 m_Position;
        glm::vec3 m_Direction;
        f32 m_OuterCos;
    };

}
#endif /* End of Helios */