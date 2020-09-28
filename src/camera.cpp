#include <cstdio>
#include <limits>

#include <glm/gtc/matrix_transform.hpp>

#include "camera.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::normalize;

namespace Helios {

    RTCRay Camera::GenerateRay(float u, float v) const {
        vec4 pixel_pos = m_InvProjection*vec4(u, v, 0.0f, 1.0f);

        /*vec3 test = normalize(vec3(m_InvProjection*vec4(0.0f, 0.0f, 0.0f, 1.0f)));
        vec3 test2 = m_InvView*vec4(test, 0.0f);
        printf("TEST: %f %f %f\n", test2.x, test2.y, test2.z);*/

        vec3 origin = vec3(m_View[3]);
        vec3 direction = normalize(vec3(m_InvView*vec4(pixel_pos.x, pixel_pos.y, pixel_pos.z, 0.0f)));
        

        RTCRay ray;
        ray.org_x = origin.x;
        ray.org_y = origin.y;
        ray.org_z = origin.z;
        ray.tnear = 0.0f;
        ray.dir_x = direction.x;
        ray.dir_y = direction.y;
        ray.dir_z = direction.z;
        ray.tfar = std::numeric_limits<float>::infinity();
        ray.mask = -1;
        ray.flags = 0;
        
        return ray;
    }

    PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float near, float far):
        Camera(glm::perspective(fov, aspect, near, far))
    {}

}