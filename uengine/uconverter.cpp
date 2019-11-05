#include "uconverter.h"

constexpr double xFit_1931(double wave)
{
    double t1 = (wave-442.0) * ((wave<442.0)?0.0624:0.0374);
    double t2 = (wave-599.8) * ((wave<599.8)?0.0264:0.0323);
    double t3 = (wave-501.1) * ((wave<501.1)?0.0490:0.0382);

    return 0.362 * std::exp(-0.5 * t1 * t1) + 1.056 * std::exp(-0.5 * t2 * t2) - 0.065 * std::exp(-0.5 * t3 * t3);
}

constexpr double yFit_1931(double wave)
{
    double t1 = (wave-568.8) * ((wave<568.8)?0.0213:0.0247);
    double t2 = (wave-530.9) * ((wave<530.9)?0.0613:0.0322);

    return 0.821 * std::exp(-0.5 * t1 * t1) + 0.286 * std::exp(-0.5 * t2 * t2);
}

constexpr double zFit_1931(double wave)
{
    double t1 = (wave-437.0) * ((wave<437.0)?0.0845:0.0278);
    double t2 = (wave-459.0) * ((wave<459.0)?0.0385:0.0725);

    return 1.217 * std::exp(-0.5 * t1 * t1) + 0.681 * std::exp(-0.5 * t2 * t2);
}

constexpr glm::dvec3 xyzSumX()
{
    glm::dvec3 sum = glm::dvec3(0);

    for(size_t l = 0; l < 123; l++)
    {
        sum.r += xFit_1931(626+l);
        sum.g += xFit_1931(503+l);
        sum.b += xFit_1931(380+l);
    }

    return sum;
}

constexpr glm::dvec3 xyzSumY()
{
    glm::dvec3 sum = glm::dvec3(0);

    for(size_t l = 0; l < 123; l++)
    {
        sum.r += yFit_1931(626+l);
        sum.g += yFit_1931(503+l);
        sum.b += yFit_1931(380+l);
    }

    return sum;
}

constexpr glm::dvec3 xyzSumZ()
{
    glm::dvec3 sum = glm::dvec3(0);

    for(size_t l = 0; l < 123; l++)
    {
        sum.r += zFit_1931(626+l);
        sum.g += zFit_1931(503+l);
        sum.b += zFit_1931(380+l);
    }

    return sum;
}

constexpr const glm::dvec3 sumX = xyzSumX();
constexpr const glm::dvec3 sumY = xyzSumY();
constexpr const glm::dvec3 sumZ = xyzSumZ();

glm::dvec3 UConverter::radianceToRGB(const glm::dvec3& radiance, URgbFormat format, double gamma) noexcept
{
    auto XYZ = radianceToXYZ(radiance);

    switch(format)
    {
    case URgbFormat::sRGB:
        return sRGB(XYZ, gamma);
    default:
        return glm::dvec3(0, 0, 0);
    }
}

glm::dvec3 UConverter::radianceToXYZ(const glm::dvec3& radiance) noexcept
{
    /*considering wavelengths from 380 to 749, splitting into 3 bins, each 123 nm long
    * sampling x y z functions at 1nm intervals*/

    double X = glm::dot(radiance / 123.0, sumX);
    double Y = glm::dot(radiance / 123.0, sumY);
    double Z = glm::dot(radiance / 123.0, sumZ);

    return glm::dvec3(X, Y, Z);
}

glm::dvec3 UConverter::sRGB(const glm::dvec3& xyz, double gamma) noexcept
{
    glm::dmat3x3 T(
                     3.2404542, -1.5371385, -0.4985314,
                     -0.9692660, 1.8760108, 0.0415560,
                     0.0556434, -0.2040259, 1.0572252
                         );

    /*linear transform from XYZ to sRGB*/
    glm::dvec3 rgb = xyz*T;

    rgb = glm::clamp(rgb, 0.0, 1.0);

    /*gamma correction*/
    for(size_t c = 0; c < 3; c++)
    {
        if(rgb[c] <= 0.0031308)
        {
            rgb[c] *= 12.92;
        }
        else
        {
            rgb[c] = 1.055 * std::pow(rgb[c], 1.0 / gamma) - 0.055;
        }
    }

    return glm::clamp(rgb, 0.0, 1.0);
}
