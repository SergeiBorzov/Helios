#ifndef HELIOS_MATERIAL_H
#define HELIOS_MATERIAL_H

#include "spectrum.h"
#include "RayHitRecord.h"

namespace Helios {
    class Material {
    public:
        // Creates and put BSDF to RayhitRecord
        virtual void ProduceBSDF(RayHitRecord& record) const = 0;
        virtual ~Material() {}
    };

    class Matte: public Material {
    public:
        Matte(const Spectrum& diffuse): m_Diffuse(diffuse) {}
        void ProduceBSDF(RayHitRecord& record) const;
        ~Matte() {}
    private:
        Spectrum m_Diffuse;
    };
}

#endif /* HELIOS_MATERIAL_H */