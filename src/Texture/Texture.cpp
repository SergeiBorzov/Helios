#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "../Core/ColorSpaces.h"
#include "Texture.h"

namespace Helios {

    Spectrum Texture::operator()(f32 u, f32 v) const {
        assert(m_Data.size() > 0);

        if (u < 0.0f) {
            u = 1.0f - u;
        }
        if (v < 0.0f) {
            v = 1.0f - v;
        }

        int x = static_cast<int>(u*m_Width) % m_Width;
        int y = (m_Height - 1) - static_cast<int>(v*m_Height) % m_Height;

        int start_index = y*m_Width*m_NumChannels + x*m_NumChannels;
        switch (m_NumChannels) {
            case 1: {
                f32 value = m_Data[start_index];
                return { value, value, value };
            }
            case 2: {
                f32 value_1 = m_Data[start_index + 0];
                f32 value_2 = m_Data[start_index + 1];
                return { value_1, value_2, 0.0f};
            }
            case 3: {
                f32 value_1 = m_Data[start_index + 0];
                f32 value_2 = m_Data[start_index + 1];
                f32 value_3 = m_Data[start_index + 2];
                return { value_1, value_2, value_3 };
                break;
            }
            case 4:  {
                f32 value_1 = m_Data[start_index + 0];
                f32 value_2 = m_Data[start_index + 1];
                f32 value_3 = m_Data[start_index + 2];
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

    std::shared_ptr<Texture> Texture::LoadFromFile(const char* path_to_file, ColorSpace color_space) {
        i32 width, height, channels;
        unsigned char* data = stbi_load(path_to_file, &width, &height, &channels, 0);

        if (!data) {
            return nullptr;
        }

        std::shared_ptr<Texture> result = std::make_shared<Texture>();
        result->m_Width = width;
        result->m_Height = height;
        result->m_NumChannels = channels;

        std::vector<unsigned char> image_data(data, data + width*height*channels);

        switch(color_space) {
            case ColorSpace::Linear: {
                convert_linear_u8_to_linear(image_data, result->m_Data);
                break;
            }
            case ColorSpace::sRGB: {
                convert_srgb_to_linear(image_data, result->m_Data);
                break;
            }
        }

        stbi_image_free(data);
        return result;
    }
}