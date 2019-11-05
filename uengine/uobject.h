#ifndef UOBJECT_H
#define UOBJECT_H

#include "uutils.h"
#include "ugeometry.h"

class UObject
{
public:
    virtual ~UObject() = default;

    /*if ray intersects the object return true and return structure with the intersection point data
     *  and the distance from the intersection point*/
    virtual bool intersectionPoint(const URay&, USurfacePoint&, double& d) = 0;
    /*if ray intersects the object return true and return distance between the
    * ray's origin and the intersection point*/
    virtual bool intersects(const URay&, double&) = 0;
    virtual bool isEmitter() const { return false; }
};

#endif // UOBJECT_H
