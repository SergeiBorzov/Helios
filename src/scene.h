#ifndef HELIOS_SCENE_H
#define HELIOS_SCENE_H

#include <vector>

#include <embree3/rtcore.h>

#include "camera.h"
#include "light.h"
#include "mesh.h"
#include "entity.h"
#include "material.h"

namespace Helios {
    class Scene {
    friend class Renderer;
    public:
        static Scene* LoadFromFile(const char* path_to_file);

        void Create();
        inline RTCScene GetRTCScene() const { return m_Scene; }
        inline void PushCamera(const Camera& camera) { m_Cameras.push_back(camera); }
        inline void PushLight(const Light& light) { m_Lights.push_back(light); }
        inline void PushEntity(const Entity& entity) { m_Entities.push_back(entity); };
        inline void PushMaterial(Material* material) { m_Materials.push_back(material); }

        ~Scene();
    private:
        std::vector<Entity> m_Entities;
        std::vector<Camera> m_Cameras;
        std::vector<Light> m_Lights;
        std::vector<Material*> m_Materials;

        RTCScene m_Scene;
    };
}

#endif /* End of HELIOS_SCENE_H */