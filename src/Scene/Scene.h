#ifndef HELIOS_SCENE_H
#define HELIOS_SCENE_H

#include <vector>
#include <unordered_map>

#include <embree3/rtcore.h>

#include "../Core/RayHitRecord.h"
#include "../Camera/Camera.h"
#include "../Light/Light.h"
#include "../Material/Material.h"
#include "Mesh.h"

namespace Helios {
    class Scene {
    friend class Integrator;
    public:
        static Scene* LoadFromFile(const char* path_to_file);

        void Create();
        bool Intersect(const RTCRay& ray, RayHitRecord& ray_hit_record) const;

        inline RTCScene GetRTCScene() const { return m_Scene; }

        inline void PushCamera(const Camera& camera) { m_Cameras.push_back(camera); }
        inline void PushLight(Light* light) { m_Lights.push_back(light); }
        inline void PushEntity(unsigned int geometry_id, unsigned int material_id) { m_MaterialMap.insert({geometry_id, material_id}); };
        inline void PushMaterial(Material* material) { m_Materials.push_back(material); }


        ~Scene();
    private:
        std::unordered_map<unsigned int, unsigned int> m_MaterialMap;
        std::vector<Camera> m_Cameras;
        std::vector<Light*> m_Lights;
        std::vector<Material*> m_Materials;

        RTCScene m_Scene;
    };
}

#endif /* End of HELIOS_SCENE_H */