#ifndef MODEL_H
#define MODEL_H

#include <ugeometry.h>
#include <uemitter.h>

class Model
{
public:
    virtual bool localIntersection(const URay& rayL, USurfacePoint& sp, double& d) = 0;
    virtual bool intersects(const URay& rayL, double& d) = 0;
    virtual double area(const glm::dmat4x4& W) = 0;
    virtual void localRandomPoint(UEmitterPoint& ep) = 0;
};

#endif // MODEL_H
