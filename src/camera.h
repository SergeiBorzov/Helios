#ifndef HELIOS_CAMERA_H
#define HELIOS_CAMERA_H

#include <Eigen/Core>

#include "ray.h"

namespace Helios {
    // base class for all cameras
    class Camera {
    public:
        Camera(const Eigen::Matrix4f& projection):
            m_Projection(projection),
            m_InvProjection(projection.inverse()),
            m_View(Eigen::Matrix4f::Identity()),
            m_InvView(Eigen::Matrix4f::Identity())
        {}

        inline void SetView(const Eigen::Matrix4f& view) { m_View = view; m_InvView = view.inverse(); }
        Ray GenerateRay(float u, float v) const;

        static Eigen::Matrix4f Perspective(float fov, float aspect, float near, float far);
    protected:
        Eigen::Matrix4f m_Projection;
        Eigen::Matrix4f m_InvProjection;
        Eigen::Matrix4f m_View;
        Eigen::Matrix4f m_InvView;
    };

    class PerspectiveCamera: public Camera {
    public:
        PerspectiveCamera(float fov, float aspect, float near, float far):
            Camera(Camera::Perspective(fov, aspect, near, far))
        {}
    private:
        //float m_FOV;
        //float m_Aspect;
        //float m_Near;
        //float m_Far;
    };
}


#endif /* End of HELIOS_CAMERA_H */