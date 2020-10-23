#ifndef HELIOS_SCENE_H
#define HELIOS_SCENE_H

#include <vector>
#include <string_view>
#include <unordered_map>

#include <embree3/rtcore.h>

#include "../Core/Types.h"
#include "../Core/RayHitRecord.h"
#include "../Camera/Camera.h"
#include "../Light/Light.h"
#include "../Material/Material.h"
#include "../Texture/Texture.h"
#include "TriangleMesh.h"

namespace Helios {
    class Scene {
    friend class Integrator;
    public:
        static Scene* LoadFromFile(const char* path_to_file, i32 width, i32 height);

        void Create();
        bool Intersect(const RTCRay& ray, RayHitRecord& ray_hit_record) const;
        bool Occluded(RTCRay& ray) const;

        inline RTCScene GetRTCScene() const { return m_Scene; }

        inline void AddCamera(const std::shared_ptr<Camera>& camera) { m_Cameras.push_back(camera); }
        inline void AddLight(const std::shared_ptr<Light>& light) { m_Lights.push_back(light); }
        inline void AddEntity(const TriangleMesh& mesh, u32 material_id);
        inline void AddMaterial(u32 index, const std::shared_ptr<Material>& material) {
            if (index >= m_Materials.size()) {
                m_Materials.resize(index + 1);
            } 
            m_Materials[index] = material; 
        }

        inline void AddTexture(const char* name, const std::shared_ptr<Texture>& texture) { m_TextureMap.insert({name, texture}); }
        inline bool HasTexture(const char* name) const { return m_TextureMap.find(name) != m_TextureMap.end(); }
        inline std::shared_ptr<Texture> GetTexture(const char* name) const { 
            auto it = m_TextureMap.find(name);
            if (it != m_TextureMap.end()) {
                return it->second;
            }
            else {
                return nullptr;
            }
        }

        ~Scene();
    private:
        std::vector<std::shared_ptr<Camera>> m_Cameras;
        std::vector<std::shared_ptr<Light>> m_Lights;
        std::vector<std::shared_ptr<Material>> m_Materials;
        std::unordered_map<u32, TriangleMesh> m_TriangleMeshes;
        std::unordered_map<u32, u32> m_MaterialMap;
        std::unordered_map<std::string_view, std::shared_ptr<Texture>> m_TextureMap; 
        RTCScene m_Scene;
    };
}

#endif /* End of HELIOS_SCENE_H */