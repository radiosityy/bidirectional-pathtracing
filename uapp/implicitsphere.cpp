#include "implicitsphere.h"

bool ImplicitSphere::localIntersection(const URay& rayL, USurfacePoint& sp, double& d)
{
    if(!rayL.intersectUnitSphere(d))
        return false;

    sp.tex_u = 0;
    sp.tex_v = 0;

    sp.pos = rayL.origin() + d*rayL.dir();
    sp.Ns = sp.Ng = glm::normalize(sp.pos);

    sp.Ts = glm::normalize(-sp.Ns + glm::dvec3(0, 0, 1.0 / sp.Ns.z));
    sp.Bs = glm::normalize(glm::cross(sp.Ns, sp.Ts));

    return true;
}

bool ImplicitSphere::intersects(const URay& rayL, double& d)
{
    return rayL.intersectUnitSphere(d);
}

double ImplicitSphere::area(const glm::dmat4x4& W)
{
    double R = glm::length(glm::dvec3(W * glm::dvec4(1, 0, 0, 0)));

    return 4.0 * M_PI * R * R;
}

void ImplicitSphere::localRandomPoint(UEmitterPoint& ep)
{
    ep.Ns = ep.Ng = URng::get().sampleUnitSphereUniform();
    ep.pos = ep.Ng ;
    ep.Ts = -ep.Ns + glm::dvec3(0, 0, 1.0 / ep.Ns.z);
    ep.Bs = glm::cross(ep.Ns, ep.Ts);
}
