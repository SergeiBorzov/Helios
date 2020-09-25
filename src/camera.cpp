#include "camera.h"

using Eigen::Matrix4f;
using Eigen::Vector3f;
using Eigen::Vector4f;

namespace Helios {
    Matrix4f Camera::Perspective(float fov, float aspect, float near, float far) {
        Matrix4f mat;

        float tan_half = tan(fov*0.5f);
        float range = far - near;

        mat << 1.0f/(tan_half*aspect), 0.0f, 0.0f, 0.0f,
               0.0f, 1.0f/tan_half, 0.0f, 0.0f,
               0.0f, 0.0f, -(near + far)/range, -2.0f*near*far/range,
               0.0f, 0.0f, -1.0f, 0.0f;

        return mat;
    }

    Ray Camera::GenerateRay(float u, float v) const {
        Vector4f pixel_pos = m_InvProjection*Vector4f(u, v, 0.0f, 1.0f);
        Vector3f dir = Vector3f(pixel_pos.x(), pixel_pos.y(), pixel_pos.z());
        
        Ray ray;
        ray.origin = m_View.col(3).head<3>();
        Vector4f dir_world_space = m_InvView*Vector4f(dir.x(), dir.y(), dir.z(), 0.0f);
        ray.direction = dir_world_space.head<3>();
        
        return ray;
    }
}