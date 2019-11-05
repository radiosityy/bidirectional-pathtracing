#ifndef TEXTURECOLOR_H
#define TEXTURECOLOR_H

#include <umath.h>
#include <utexture.h>

class TextureColor : public UTexture
{
public:
    TextureColor(const glm::dvec3 color);
    glm::dvec3 sample(double u, double v) override;

private:
    glm::dvec3 m_color;
};

#endif // TEXTURECOLOR_H
