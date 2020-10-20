#ifndef HELIOS_COLOR_SPACES_H
#define HELIOS_COLOR_SPACES_H

#include <vector>


namespace Helios {
    void convert_linear_u8_to_linear(const std::vector<unsigned char>& rgb, std::vector<float>& lrgb);
    void convert_srgb_to_linear(const std::vector<unsigned char>& srgb, std::vector<float>& lrgb);
    void convert_linear_to_srgb(const std::vector<float>& linear, std::vector<unsigned char>& srgb);
}



#endif /* End of HELIOS_CONVERTER_H */