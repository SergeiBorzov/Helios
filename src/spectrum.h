#ifndef HELIOS_SPECTRUM_H
#define HELIOS_SPECTRUM_H

namespace Helios {
    
    // Helios uses linear sRGB color space
    struct Spectrum {
        inline Spectrum operator*(float value) { return { r*value, b*value, g*value }; }

        float r;
        float g;
        float b;
    };

}

#endif /* End of HELIOS_SPECTRUM_H */