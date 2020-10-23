#ifndef HELIOS_COLOR_SPACES_H
#define HELIOS_COLOR_SPACES_H

#include <vector>

#include "Types.h"

namespace Helios {
    void convert_linear_u8_to_linear(const std::vector<u8>& rgb, std::vector<f32>& lrgb);
    void convert_srgb_to_linear(const std::vector<u8>& srgb, std::vector<f32>& lrgb);
    void convert_linear_to_srgb(const std::vector<f32>& linear, std::vector<u8>& srgb);
}



#endif /* End of HELIOS_CONVERTER_H */