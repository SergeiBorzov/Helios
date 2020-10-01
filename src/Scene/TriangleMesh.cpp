#include "../Core/Globals.h"
#include "TriangleMesh.h"

using namespace glm;

namespace Helios {
    void TriangleMesh::Create(std::vector<vec3>&& vertices_in,
                              std::vector<unsigned int>&& indices_in, 
                              std::vector<vec3>&& normals_in,
                              std::vector<vec2>&& uvs_in) {
        m_Geometry = rtcNewGeometry(g_Device, RTC_GEOMETRY_TYPE_TRIANGLE);

        // Create and buffer vertex buffer
        vec3* vertices = 
            (vec3*)rtcSetNewGeometryBuffer(m_Geometry, 
                                            RTC_BUFFER_TYPE_VERTEX, 
                                            0, RTC_FORMAT_FLOAT3, 
                                            sizeof(vec3), 
                                            vertices_in.size());
        memcpy(vertices, vertices_in.data(), sizeof(vec3)*vertices_in.size());

        // Create and buffer index buffer
        unsigned int* indices = 
            (unsigned int*)rtcSetNewGeometryBuffer(m_Geometry, 
                                                   RTC_BUFFER_TYPE_INDEX, 
                                                   0, RTC_FORMAT_UINT3, 
                                                   3*sizeof(unsigned int), 
                                                   indices_in.size() / 3);
        memcpy(indices, indices_in.data(), sizeof(unsigned int)*indices_in.size());
        
        /*
        if (!normals_in.empty())
            attribute_count++;
        if (!uvs_in.empty())
                attribute_count;
        */
        unsigned int attribute_count = 0;
        if (!normals_in.empty()) {
           ++attribute_count;
           m_HasNormals = true;
        }

        if (!uvs_in.empty()) {
            ++attribute_count;
            m_HasUVs = true;
        }

        rtcSetGeometryVertexAttributeCount(m_Geometry, attribute_count);
        // Create and buffer normals
        if (!normals_in.empty()) {
            vec3* normals = 
                (vec3*)rtcSetNewGeometryBuffer(m_Geometry,
                                               RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
                                               0, RTC_FORMAT_FLOAT3,
                                               sizeof(vec3), normals_in.size());
            memcpy(normals, normals_in.data(), sizeof(vec3)*normals_in.size());
        }

        // Create and buffer uvs
        if (!uvs_in.empty()) {
            vec2* uvs = 
                (vec2*)rtcSetNewGeometryBuffer(m_Geometry,
                                               RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
                                               1, RTC_FORMAT_FLOAT2,
                                               sizeof(vec2), uvs_in.size());
            memcpy(uvs, uvs_in.data(), sizeof(vec2)*uvs_in.size());
        }
           
        rtcCommitGeometry(m_Geometry);
    }

    void TriangleMesh::Release() {
        rtcReleaseGeometry(m_Geometry);
    }
}