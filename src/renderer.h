#ifndef HELIOS_RENDERER_H
#define HELIOS_RENDERER_H

#include <embree3/rtcore.h>

namespace Helios {
    class Renderer {
    public:
        bool Init();
        ~Renderer();
    private:
        RTCDevice m_Device = 0;
    };
}

#endif /* End of HELIOS_RENDERER_H */