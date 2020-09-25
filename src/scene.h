#ifndef HELIOS_SCENE_H
#define HELIOS_SCENE_H

#include <vector>

#include <embree3/rtcore.h>

#include "camera.h"
#include "light.h"
#include "mesh.h"

namespace Helios {
    class Scene {
    friend class Renderer;
    public:
        static Scene* LoadFromFile(const char* path_to_file);

        void Create();
        inline RTCScene GetRTCScene() const { return m_Scene; }
        inline void PushCamera(const Camera& camera) { m_Cameras.push_back(camera); }
        inline void PushLight(const Light& light) { m_Lights.push_back(light); }
        inline void PushGeometryID(const unsigned int object_id) { m_Objects.push_back(object_id); };

        ~Scene();
    private:
        std::vector<unsigned int> m_Objects;
        std::vector<Camera> m_Cameras;
        std::vector<Light> m_Lights;

        RTCScene m_Scene;
    };
}

#endif /* End of HELIOS_SCENE_H */