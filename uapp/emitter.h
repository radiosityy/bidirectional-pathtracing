#ifndef EMITTER_H
#define EMITTER_H

#include <uemitter.h>
#include "object.h"

class Emitter : public Object, public UEmitter
{
public:
    Emitter(std::shared_ptr<Model> model, const glm::dmat4x4& W, const std::shared_ptr<Material>& mat, const glm::dvec3& power);

    glm::dvec3 power() override;
    double area() override;
    void randomPoint(UEmitterPoint&) override;

private:
    glm::dvec3 m_P; //emitted power
    double m_A; //surface area
};

#endif // EMITTER_H
