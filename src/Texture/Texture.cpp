#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../Core/ColorSpaces.h"
#include "Texture.h"

namespace Helios {

    Spectrum Texture::operator()(float u, float v) const {
        assert(m_Data.size() > 0);

        int x = static_cast<int>(u*m_Width) % m_Width;
        int y = m_Height - static_cast<int>(v*m_Height) % m_Height;

        int start_index = y*m_Width*m_NumChannels + x*m_NumChannels;
        switch (m_NumChannels) {
            case 1: {
                float value = m_Data[start_index];
                return { value, value, value };
            }
            case 2: {
                float value_1 = m_Data[start_index + 0];
                float value_2 = m_Data[start_index + 1];
                return { value_1, value_2, 0.0f};
            }
            case 3: {
                float value_1 = m_Data[start_index + 0];
                float value_2 = m_Data[start_index + 1];
                float value_3 = m_Data[start_index + 2];
                return { value_1, value_2, value_3 };
                break;
            }
            case 4:  {
                float value_1 = m_Data[start_index + 0];
                float value_2 = m_Data[start_index + 1];
                float value_3 = m_Data[start_index + 2];
                return { value_1, value_2, value_3 };
                break;
            }
            default: {
                assert(false);
                break;
            }
        }
        return {0.0f, 0.0f, 0.0f};
    }

    bool Texture::LoadFromFile(const char* path_to_file, ColorSpace color_space) {
        unsigned char* data = stbi_load(path_to_file, &m_Width, &m_Height, &m_NumChannels, 0);

        if (!data) {
            return false;
        }

        std::vector<unsigned char> image_data(data, data + m_Width*m_Height*m_NumChannels);

        switch(color_space) {
            case ColorSpace::Linear: {
                convert_linear_u8_to_linear(image_data, m_Data);
                break;
            }
            case ColorSpace::sRGB: {
                convert_srgb_to_linear(image_data, m_Data);
                break;
            }
        }

        stbi_image_free(data);
        return true;
    }
}