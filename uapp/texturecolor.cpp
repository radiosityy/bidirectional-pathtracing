#include "texturecolor.h"

TextureColor::TextureColor(const glm::dvec3 color)
{
    m_color = color;
}

glm::dvec3 TextureColor::sample(double u, double v)
{
    return m_color;
}
