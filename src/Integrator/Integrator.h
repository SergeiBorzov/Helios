#ifndef HELIOS_RENDERER_H
#define HELIOS_RENDERER_H

#include <vector>

#include <embree3/rtcore.h>

#include "../Core/Spectrum.h"
#include "../Scene/Scene.h"

namespace Helios {

    class Integrator {
    public:
        void Render(std::vector<float>& image_lrgb, const Scene& scene, int width, int height);
    private:
        Spectrum RayTracing(const Scene& scene, const RTCRay& ray);
    };
}

#endif /* End of HELIOS_RENDERER_H */