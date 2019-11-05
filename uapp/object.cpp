#include "object.h"

#include <uengine.h>

Object::Object(std::shared_ptr<Model> model, const glm::dmat4x4& W, const std::shared_ptr<Material>& mat)
{
    m_W = W;
    m_invW = glm::inverse(W);

    m_material = mat;
    m_model = model;
}

bool Object::intersectionPoint(const URay& ray, USurfacePoint& sp, double& d)
{
    if(!m_model->localIntersection(ray.transform(m_invW), sp, d))
        return false;

    sp.W = m_W;
    sp.invW = m_invW;

    sp.bsdf = m_material->bsdf();

    return true;
}

bool Object::intersects(const URay& ray, double& d)
{
    return m_model->intersects(ray.transform(m_invW), d);
}
