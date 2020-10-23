#ifndef HELIOS_SPECTRUM_H
#define HELIOS_SPECTRUM_H

#include "Types.h"

namespace Helios {
    
    // Helios uses linear sRGB color space
    struct Spectrum {
        Spectrum(f32 red, f32 green, f32 blue): r(red), g(green), b(blue) {} 
        Spectrum operator*(f32 value) const;
        Spectrum operator/(f32 value) const;
        Spectrum operator*(const Spectrum& rhs) const;
        Spectrum operator+(const Spectrum& rhs) const;
        Spectrum& operator+=(const Spectrum& rhs);
        Spectrum& operator*=(const Spectrum& rhs);

        f32 r;
        f32 g;
        f32 b;
    };

    inline Spectrum Spectrum::operator*(f32 value) const {
        return { r*value, b*value, g*value };
    }

    inline Spectrum Spectrum::operator/(f32 value) const {
        return { r/value, b/value, g/value };
    }

    inline Spectrum Spectrum::operator*(const Spectrum& rhs) const {
        return { this->r*rhs.r, this->g*rhs.g, this->b*rhs.b };
    }

    inline Spectrum Spectrum::operator+(const Spectrum& rhs) const {
        return { this->r + rhs.r, this->g + rhs.g, this->b + rhs.b };
    }

    inline Spectrum& Spectrum::operator+=(const Spectrum& rhs) {
        this->r += rhs.r; 
        this->g += rhs.g; 
        this->b += rhs.b; 
        return *this;
    }

    inline Spectrum& Spectrum::operator*=(const Spectrum& rhs) {
        this->r *= rhs.r; 
        this->g *= rhs.g; 
        this->b *= rhs.b; 
        return *this;
    }

}

#endif /* End of HELIOS_SPECTRUM_H */