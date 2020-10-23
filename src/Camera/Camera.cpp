#include <cstdio>
#include <limits>

#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

using namespace glm;

namespace Helios {

    RTCRay Camera::GenerateRay(f32 u, f32 v) const {
        vec4 pixel_pos = m_InvProjection*vec4(u, v, 0.0f, 1.0f);

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
        ray.tfar = std::numeric_limits<f32>::infinity();
        ray.mask = -1;
        ray.flags = 0;
        
        return ray;
    }

    PerspectiveCamera::PerspectiveCamera(f32 fov, f32 aspect, f32 near, f32 far):
        Camera(glm::perspective(fov, aspect, near, far))
    {}

}