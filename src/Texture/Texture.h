#ifndef HELIOS_TEXTURE_H
#define HELIOS_TEXTURE_H

#include <memory>

#include "../Core/Spectrum.h"

namespace Helios {
    class Texture {
    public:
        bool LoadFromFile(const char* path_to_file);

        inline int GetWidth() const { return m_Width; }
        inline int GetHeight() const { return m_Height; }
        inline int GetNumChannels() const { return m_NumChannels; }
        
        inline bool HasAlpha() const { return m_NumChannels == 4; }

        Spectrum operator()(float u, float v) const;

        virtual ~Texture();
    private:
        unsigned char* m_Data = nullptr;
        int m_Width = 0;
        int m_Height = 0;
        int m_NumChannels = 0;
    };

}

#endif /* End of HELIOS_TEXTURE_H */