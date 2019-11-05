#ifndef IMPLICITSPHERE_H
#define IMPLICITSPHERE_H

#include "model.h"

class ImplicitSphere : public Model
{
public:
    bool localIntersection(const URay& rayL, USurfacePoint& sp, double& d) override;
    bool intersects(const URay& rayL, double& d) override;
    double area(const glm::dmat4x4& W) override;
    void localRandomPoint(UEmitterPoint&) override;
};

#endif // IMPLICITSPHERE_H
