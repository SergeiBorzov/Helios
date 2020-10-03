#ifndef HELIOS_MATERIAL_H
#define HELIOS_MATERIAL_H

#include <memory>

#include "../Core/Spectrum.h"
#include "../Core/RayHitRecord.h"
#include "../Texture/Texture.h"

namespace Helios {
    class Material {
    public:
        // Creates and put BSDF to RayhitRecord
        virtual void ProduceBSDF(RayHitRecord& record) const = 0;
        virtual ~Material() {}
    };

    class Matte: public Material {
    public:
        Matte(const Spectrum& diffuse, 
              const std::shared_ptr<Texture>& diffuse_texture = nullptr,
              const std::shared_ptr<Texture>& normal_map = nullptr): 
            m_Diffuse(diffuse),
            m_DiffuseTexture(diffuse_texture),
            m_NormalMap(normal_map) 
        {}
        void ProduceBSDF(RayHitRecord& record) const;
        ~Matte() {}
    private:
        Spectrum m_Diffuse;
        std::shared_ptr<Texture> m_DiffuseTexture = nullptr;
        std::shared_ptr<Texture> m_NormalMap = nullptr;   
    };
}

#endif /* HELIOS_MATERIAL_H */