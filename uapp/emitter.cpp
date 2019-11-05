#include "emitter.h"

Emitter::Emitter(std::shared_ptr<Model> model, const glm::dmat4x4 &W, const std::shared_ptr<Material>& mat, const glm::dvec3 &power) : Object(model, W, mat)
{
    m_P = power;
    m_A = m_model->area(m_W);
}

glm::dvec3 Emitter::power()
{
    return m_P;
}

double Emitter::area()
{
    return m_A;
}

void Emitter::randomPoint(UEmitterPoint& ep)
{
    m_model->localRandomPoint(ep);

    ep.pos += ep.Ng * 0.0001;

    ep.pos = transformPoint(m_W, ep.pos);
    ep.Ng = transformVectorT(m_invW, ep.Ng);
    ep.Ns = transformVectorT(m_invW, ep.Ns);
    ep.Ts = transformVectorT(m_invW, ep.Ts);
    ep.Bs = transformVectorT(m_invW, ep.Bs);
}
