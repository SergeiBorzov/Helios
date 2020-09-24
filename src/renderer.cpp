#include <cstdio>

#include "renderer.h"

void error_callback(void* userPtr, enum RTCError error, const char* str) {
    printf("Embree error %d: %s\n", error, str);
}

namespace Helios {
    
    bool Renderer::Init() {
        m_Device = rtcNewDevice(nullptr);

        if (m_Device) {
            rtcSetDeviceErrorFunction(m_Device, error_callback, nullptr);
        }
        return m_Device;
    }

    Renderer::~Renderer() {
        if (m_Device) {
             rtcReleaseDevice(m_Device);
        }
    }
}