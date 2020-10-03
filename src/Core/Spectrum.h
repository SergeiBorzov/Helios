#ifndef HELIOS_SPECTRUM_H
#define HELIOS_SPECTRUM_H

namespace Helios {
    
    // Helios uses linear sRGB color space
    struct Spectrum {
        Spectrum operator*(float value);
        Spectrum operator*(const Spectrum& rhs);
        Spectrum operator+(const Spectrum& rhs);
        Spectrum& operator+=(const Spectrum& rhs);
        Spectrum& operator*=(const Spectrum& rhs);

        float r;
        float g;
        float b;
    };

    inline Spectrum Spectrum::operator*(float value) {
        return { r*value, b*value, g*value };
    }

    inline Spectrum Spectrum::operator*(const Spectrum& rhs) {
        return { this->r*rhs.r, this->g*rhs.g, this->b*rhs.b };
    }

    inline Spectrum Spectrum::operator+(const Spectrum& rhs) {
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