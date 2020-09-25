#include <cassert>
#include <cstring> // for memcpy
#include <vector>

#include "globals.h"
#include "scene.h"

struct Vertex {
    float x;
    float y;
    float z;
};

struct Triangle {
    unsigned int v0;
    unsigned int v1;
    unsigned int v2;
};

namespace Helios {
    unsigned int Mesh::CreateAndAttach(const Scene& scene, const aiMesh* mesh) {
        if (mesh) {
            m_Geometry = rtcNewGeometry(g_Device, RTC_GEOMETRY_TYPE_TRIANGLE);

            // Create and buffer vertex buffer
            Vertex* vertices = (Vertex*)rtcSetNewGeometryBuffer(m_Geometry, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, sizeof(aiVector3D), mesh->mNumVertices);
            memcpy(vertices, mesh->mVertices, mesh->mNumVertices);

            // Create and buffer index buffer
            Triangle* triangles = (Triangle*)rtcSetNewGeometryBuffer(m_Geometry, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, sizeof(Triangle), mesh->mNumFaces);
            std::vector<Triangle> triangle_vector;
            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                const aiFace& face = mesh->mFaces[i];
                assert(face.mNumIndices == 3);
                triangle_vector.push_back({face.mIndices[0], face.mIndices[1], face.mIndices[2]});
            }
            memcpy(triangles, triangle_vector.data(), triangle_vector.size());
            rtcCommitGeometry(m_Geometry);

            unsigned int geometry_id = rtcAttachGeometry(scene.GetRTCScene(), m_Geometry);
            rtcReleaseGeometry(m_Geometry);

            return geometry_id;
        }

        return RTC_INVALID_GEOMETRY_ID;
    }
}