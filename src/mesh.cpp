#include <cassert>
#include <cstring> // for memcpy
#include <vector>

#include "globals.h"
#include "scene.h"

using glm::vec3;

namespace Helios {
    unsigned int CreateTriangleMesh(const Scene& scene, 
                                    std::vector<float>&& vertices_in,
                                    std::vector<unsigned int>&& indices_in) {         
            for (unsigned int i = 0; i < vertices_in.size(); i += 3) {
                printf("%f %f %f\n", vertices_in[i], vertices_in[i+1], vertices_in[i+2]);
            }   

            for (auto& index: indices_in) {
                printf("%u ", index);
            }
            printf("\n");

            RTCGeometry geometry = rtcNewGeometry(g_Device, RTC_GEOMETRY_TYPE_TRIANGLE);
            // Create and buffer vertex buffer
            float* vertices = (float*)rtcSetNewGeometryBuffer(geometry, 
                                                              RTC_BUFFER_TYPE_VERTEX, 
                                                              0, RTC_FORMAT_FLOAT3, 
                                                              3*sizeof(float), 
                                                              vertices_in.size() / 3);
            memcpy(vertices, vertices_in.data(), vertices_in.size());

            

            // Create and buffer index buffer
            unsigned int* indices = (unsigned int*)rtcSetNewGeometryBuffer(geometry, 
                                                                           RTC_BUFFER_TYPE_INDEX, 
                                                                           0, RTC_FORMAT_UINT3, 3*sizeof(unsigned int), 
                                                                           indices_in.size() / 3);
            memcpy(indices, indices_in.data(), indices_in.size());
            
            rtcCommitGeometry(geometry);
            unsigned int geometry_id = rtcAttachGeometry(scene.GetRTCScene(), geometry);
            rtcReleaseGeometry(geometry);
            return geometry_id;
    }
}