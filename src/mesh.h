#ifndef HELIOS_MESH_H
#define HELIOS_MESH_H

#include <assimp/scene.h>
#include <embree3/rtcore.h>

namespace Helios {
    class Scene;

    class Mesh {
    public:
        unsigned int CreateAndAttach(const Scene& scene, const aiMesh* mesh);
        inline RTCGeometry GetRTCGeometry() const { return m_Geometry; }
    private:
        RTCGeometry m_Geometry; // geometry handle
    };
}

#endif /* End of HELIOS_MESH_H */