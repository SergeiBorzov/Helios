#ifndef HELIOS_RENDERER_H
#define HELIOS_RENDERER_H

#include <vector>

#include <embree3/rtcore.h>

#include "spectrum.h"
#include "scene.h"

namespace Helios {

    class Renderer {
    public:
        std::vector<Spectrum> Draw(const Scene& scene, int width, int height);
    private:

    };
}

#endif /* End of HELIOS_RENDERER_H */