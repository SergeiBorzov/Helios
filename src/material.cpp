#include "material.h"
#include "bxdf.h"

namespace Helios {
    void Matte::ProduceBSDF(RayHitRecord& record) const {
        record.bsdf = std::make_shared<BSDF>(/* Record */);
        record.bsdf->Add(std::make_unique<Lambertian>(m_Diffuse));
    }
}