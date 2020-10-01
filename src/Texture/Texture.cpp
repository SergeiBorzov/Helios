#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Texture.h"

namespace Helios {

    Spectrum Texture::operator()(float u, float v) const {
        assert(m_Data);

        int x = static_cast<int>(u*m_Width);
        int y = static_cast<int>((1.0f - v)*m_Height);

        int start_index = y*m_Width*m_NumChannels + x;
        switch (m_NumChannels) {
            case 1: {
                float value = m_Data[start_index] / 255.0f;
                return { value, value, value };
            }
            case 2: {
                float value_1 = m_Data[start_index] / 255.0f;
                float value_2 = m_Data[start_index + 1] / 255.0f;
                return { value_1, value_2, 0.0f};
            }
            case 3:
            case 4:  {
                float value_1 = m_Data[start_index] / 255.0f;
                float value_2 = m_Data[start_index + 1] / 255.0f;
                float value_3 = m_Data[start_index + 2] / 255.0f;
                return { value_1, value_2, value_3 };
                break;
            }
            default: {
                assert(false);
                break;
            }
        }
    }

    bool Texture::LoadFromFile(const char* path_to_file) {
        m_Data = stbi_load(path_to_file, &m_Width, &m_Height, &m_NumChannels, 4);
        return m_Data;
    }

    Texture::~Texture() {
        if (m_Data) {
            stbi_image_free(m_Data);
        }
    }
}