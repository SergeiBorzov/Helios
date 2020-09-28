#ifndef HELIOS_RENDERER_H
#define HELIOS_RENDERER_H

#include <vector>

#include <embree3/rtcore.h>

#include "spectrum.h"
#include "scene.h"

namespace Helios {

    class Renderer {
    public:
        void Draw(std::vector<Spectrum>& buffer, const Scene& scene, int width, int height);
    private:
        Spectrum RayTracing(const Scene& scene, const RTCRay& ray);
    };
}

#endif /* End of HELIOS_RENDERER_H */