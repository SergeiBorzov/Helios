#ifndef HELIOS_CAMERA_H
#define HELIOS_CAMERA_H

#include <glm/glm.hpp>
#include <embree3/rtcore.h>

namespace Helios {
    // base class for all cameras
    class Camera {
    public:
        Camera(const glm::mat4& projection):
            m_Projection(projection),
            m_InvProjection(glm::inverse(projection)),
            m_View(glm::mat4(1.0f)),
            m_InvView(glm::mat4(1.0f))
        {}

        inline void SetView(const glm::mat4& view) { m_View = view; m_InvView = glm::inverse(view); }
        RTCRay GenerateRay(float u, float v) const;

    protected:
        glm::mat4 m_Projection;
        glm::mat4 m_InvProjection;
        glm::mat4 m_View;
        glm::mat4 m_InvView;
    };

    class PerspectiveCamera: public Camera {
    public:
        PerspectiveCamera(float fov, float aspect, float near, float far);
    private:
        //float m_FOV;
        //float m_Aspect;
        //float m_Near;
        //float m_Far;
    };
}


#endif /* End of HELIOS_CAMERA_H */