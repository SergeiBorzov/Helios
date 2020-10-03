#ifndef HELIOS_TRIANGLE_MESH_H
#define HELIOS_TRIANGLE_MESH_H

#include <vector>

#include <embree3/rtcore.h>
#include <glm/glm.hpp>

namespace Helios {
    class TriangleMesh {
    public:
        void Create(std::vector<glm::vec3>&& vertices,
                    std::vector<unsigned int>&& indices,
                    std::vector<glm::vec3>&& normals,
                    std::vector<glm::vec2>&& uvs,
                    std::vector<glm::vec3>&& tangents);         
        inline RTCGeometry GetRTCGeometry() const { return m_Geometry; }
        inline bool HasNormals() const { return m_HasNormals; }
        inline bool HasUVs() const { return m_HasUVs; }
        void Release();
    private:
        RTCGeometry m_Geometry = nullptr;
        bool m_HasNormals = false;
        bool m_HasUVs = false; 
    };
}

#endif /* End of HELIOS_TRIANGLE_MESH_H */