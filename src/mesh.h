#ifndef HELIOS_MESH_H
#define HELIOS_MESH_H

#include <vector>

#include <glm/glm.hpp>
#include <embree3/rtcore.h>


namespace Helios {
    class Scene;

    unsigned int CreateTriangleMesh(const Scene& scene, 
                                    std::vector<float>&& vertices, 
                                    std::vector<unsigned int>&& indices);
}

#endif /* End of HELIOS_MESH_H */