#ifndef UEMITTER_H
#define UEMITTER_H

#include "uobject.h"

struct UEmitterPoint
{
    glm::dvec3 pos;
    glm::dvec3 Ng;
    glm::dvec3 Ns;
    glm::dvec3 Ts;
    glm::dvec3 Bs;
};

class UEmitter : public virtual UObject
{
public:
    /*returns the emitter's total emitted power*/
    virtual glm::dvec3 power() = 0;
    /*returns the emitter's surface area*/
    virtual double area() = 0;
    /*returns a random point on the emitter's surface*/
    virtual void randomPoint(UEmitterPoint&) = 0;
    bool isEmitter() const override { return true; }

    double probability() const { return m_p; }
    void setProbability(double p) { m_p = p; }
private:
    double m_p;
};

#endif // UEMITTER_H
