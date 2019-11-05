#include "ubsdf.h"

#include "uutils.h"

UBsdfSurfaceInfo UBsdfSurfaceInfo::fromSurfacePoint(const USurfacePoint& sp)
{
    UBsdfSurfaceInfo info;

    info.Ng = sp.Ng;
    info.Ns = sp.Ns;
    info.Ts = sp.Ts;
    info.Bs = sp.Bs;
    info.eta_t = 1.0;
    info.tex_u = sp.tex_u;
    info.tex_v = sp.tex_v;

    return info;
}
