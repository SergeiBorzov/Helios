#include <glm/glm.hpp>

#include "ColorSpaces.h"

namespace Helios {
    void convert_linear_u8_to_linear(const std::vector<unsigned char>& rgb, std::vector<float>& lrgb) {
        lrgb.resize(rgb.size());

        for (int i = 0; i < rgb.size(); i++) {
            lrgb[i] = rgb[i] / 255.0f;
        }
    }

    void convert_srgb_to_linear(const std::vector<unsigned char>& srgb, std::vector<float>& lrgb) {
        lrgb.resize(srgb.size());

        for (int i = 0; i < lrgb.size(); i++) {
            lrgb[i] = srgb[i] / 255.0f;
        }

        for (int i = 0; i < srgb.size(); i++) {
            if (lrgb[i] <= 0.04045f) {
                lrgb[i] /= 12.92f;
            }
            else {
                lrgb[i] = glm::pow((lrgb[i] + 0.055f) / 1.055f, 2.4f);
            }
        }
    }

    void convert_linear_to_srgb(const std::vector<float>& lrgb, std::vector<unsigned char>& srgb) {
        srgb.resize(lrgb.size());


        for (int i = 0; i < lrgb.size(); i++) {
            if (lrgb[i] <= 0.0031308f) {
                srgb[i] = (lrgb[i]*12.92f)*255 + 0.5f;
            }
            else {
                srgb[i] = (1.055 * glm::pow(lrgb[i], 1.0 / 2.4) - 0.055)*255 + 0.5f;
            }
        }
    }
}