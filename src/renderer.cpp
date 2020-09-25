#include <cstdio>

#include "renderer.h"

namespace Helios {
    std::vector<Spectrum> Renderer::Draw(const Scene& scene, int width, int height) {
        std::vector<Spectrum> buffer;
        buffer.resize(width*height);

        // Render image from each camera of the scene
        for (const auto& camera: scene.m_Cameras) {
            camera.GenerateRay(0.0f, 0.0f);
        }

        return buffer;
    }
}