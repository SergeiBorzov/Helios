#include "../Core/Globals.h"
#include "TriangleMesh.h"

using namespace glm;

namespace Helios {
    void TriangleMesh::Create(std::vector<vec3>&& vertices_in,
                              std::vector<u32>&& indices_in, 
                              std::vector<vec3>&& normals_in,
                              std::vector<vec2>&& uvs_in,
                              std::vector<vec3>&& tangents_in) {
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
        u32* indices = 
            (u32*)rtcSetNewGeometryBuffer(m_Geometry, 
                                                   RTC_BUFFER_TYPE_INDEX, 
                                                   0, RTC_FORMAT_UINT3, 
                                                   3*sizeof(u32), 
                                                   indices_in.size() / 3);
        memcpy(indices, indices_in.data(), sizeof(u32)*indices_in.size());
       
        u32 attribute_count = 0;
        if (!normals_in.empty()) {
           ++attribute_count;
           m_HasNormals = true;
        }

        if (!uvs_in.empty()) {
            ++attribute_count;
            m_HasUVs = true;
        }

        if (!tangents_in.empty()) {
            ++attribute_count;
        }
        else if (m_HasNormals && m_HasUVs) {
            // Note: if we have normals and uvs we can generate tangent for each vertex
            // Note: gltf 2.0 specification guarantees that triangle front-face has CCW order
            ++attribute_count;

            tangents_in = std::vector<vec3>(vertices_in.size(), vec3(0.0f));

            vec3* tangents = new vec3[vertices_in.size()*2];
            vec3* bitangents = tangents + vertices_in.size();

            // Calculating tangent for each triangle
            for (u32 i = 0; i < indices_in.size(); i += 3) {
                const vec3& p0 = vertices_in[indices_in[i + 0]];
                const vec3& p1 = vertices_in[indices_in[i + 1]];
                const vec3& p2 = vertices_in[indices_in[i + 2]];

                const vec2& uv0 = uvs_in[indices_in[i + 0]];
                const vec2& uv1 = uvs_in[indices_in[i + 1]];
                const vec2& uv2 = uvs_in[indices_in[i + 2]];

                // Finding tangent using following:
                // p_i - p_j = (u_i - u_j)*t + (v_i - v_j)*b

                vec2 xy1 = uv1 - uv0;
                vec2 xy2 = uv2 - uv0;
                float d = 1 / (xy1.x*xy2.y - xy2.x*xy1.y);

                mat2x3 e(p1 - p0, p2 - p0);
                mat2x2 xy(vec2(xy2.y, -xy1.y), vec2(xy2.x, -xy1.x));

                mat2x3 tb = d*e*xy;

                tangents[indices_in[i + 0]] += tb[0];
                tangents[indices_in[i + 1]] += tb[0];
                tangents[indices_in[i + 2]] += tb[0];
                bitangents[indices_in[i + 0]] += tb[1];
                bitangents[indices_in[i + 1]] += tb[1];
                bitangents[indices_in[i + 2]] += tb[1];
            }


            // Compute tangent and bitangent for each vertex using orhonormalization
            for (u32 i = 0; i < vertices_in.size(); i++) {
                const vec3& t = tangents[i];
                const vec3& b = bitangents[i];
                const vec3& n = normals_in[i];

                tangents_in[i] = normalize(t - dot(t, n)*n);
                if (dot(cross(t, b), n) < 0.0f) {
                    tangents_in[i] *= -1.0f;
                }
            }

            delete[] tangents;
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

         // Create and buffer tangents
        if (!tangents_in.empty()) {
            vec3* tangents = 
                (vec3*)rtcSetNewGeometryBuffer(m_Geometry,
                                               RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,
                                               2, RTC_FORMAT_FLOAT3,
                                               sizeof(vec3), tangents_in.size());
            memcpy(tangents, tangents_in.data(), sizeof(vec3)*tangents_in.size());
        }
           
        rtcCommitGeometry(m_Geometry);
    }

    void TriangleMesh::Release() {
        rtcReleaseGeometry(m_Geometry);
    }
}