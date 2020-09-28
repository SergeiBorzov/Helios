#ifndef HELIOS_MATERIAL_H
#define HELIOS_MATERIAL_H

#include "spectrum.h"

namespace Helios {
    class Material {
    public:
        // Creates and put BSDF to RayhitRecord
        //virtual void ProduceBSDF(RayHitRecord& record) const = 0;
    };

    class Matte: public Material {
    public:
        Matte(const Spectrum& diffuse): m_Diffuse(diffuse) {}
        //void ProduceBSDF(RayHitRecord& record) const;
    private:
        Spectrum m_Diffuse;
    };
}

#endif /* HELIOS_MATERIAL_H */