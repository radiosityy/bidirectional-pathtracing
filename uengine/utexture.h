#ifndef UTEXTURE_H
#define UTEXTURE_H

#include "umath.h"

class UTexture
{
public:
    virtual glm::dvec3 sample(double u, double v) = 0;
};

#endif // UTEXTURE_H
