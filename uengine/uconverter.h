#ifndef UCONVERTER_H
#define UCONVERTER_H

#include "umath.h"

enum class URgbFormat{sRGB};

class UConverter
{
public:
    /*computes RGB color values from computed radiance*/
    static glm::dvec3 radianceToRGB(const glm::dvec3&, URgbFormat, double gamma) noexcept;

private:
    /*converts radiance measurements to CIE XYZ values*/
    static glm::dvec3 radianceToXYZ(const glm::dvec3&) noexcept;
    /*computes RGB color values from CIE XYZ values*/
    static glm::dvec3 sRGB(const glm::dvec3&, double gamma) noexcept;
};

#endif // UCONVERTER_H
