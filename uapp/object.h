#ifndef OBJECT_H
#define OBJECT_H

#include <memory>

#include <uobject.h>
#include "model.h"
#include "material.h"

class Object : public virtual UObject
{
public:
    Object(std::shared_ptr<Model> model, const glm::dmat4x4& W, const std::shared_ptr<Material>& mat);

    virtual bool intersectionPoint(const URay&, USurfacePoint&, double& d) override;
    virtual bool intersects(const URay&, double&) override;

protected:
    glm::dmat4x4 m_W;
    glm::dmat4x4 m_invW;

    std::shared_ptr<Material> m_material;
    std::shared_ptr<Model> m_model;
};

#endif // OBJECT_H
