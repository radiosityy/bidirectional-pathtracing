#ifndef UBSDF_H
#define UBSDF_H

#include "umath.h"

struct USurfacePoint;

struct UBsdfSurfaceInfo
{
    static UBsdfSurfaceInfo fromSurfacePoint(const USurfacePoint&);

    /*all the vectors below need to be in the same coordinate space*/
    glm::dvec3 Ng;
    glm::dvec3 Ns;
    glm::dvec3 Ts;
    glm::dvec3 Bs;
    double tex_u;
    double tex_v;
    double eta_t; //refractive index of the transmitted medium
};

class UBsdf
{
public:
    /*returns the bsdf value (with respect to projected solid angle measure) for light incoming direction wiT and outgoing direction woT (both given in tangent space)*/
    virtual glm::dvec3 samplePSA(const UBsdfSurfaceInfo& info, const glm::dvec3& wiT, const glm::dvec3& woT) = 0;

    /*returns probability density (with respect to projected solid angle measure) for sampling direction wsT for given direction wgT (both given in tangent space)*/
    virtual double pPSA(const UBsdfSurfaceInfo& info, const glm::dvec3& wsT, const glm::dvec3& wgT) = 0;

    /*for given direction w (pointing away from the surface) returns:
     * scat_dirT - scattered direcion in tangent space sampled from the pPSA above
     * pPSA - the probability density with respect to projected solid angle measure for the scattered direction
     * bsdf_samplePSA - the value of the bsdf with respect to projected solid angle measure for the scattered direction
     * specular - a flag indicating whether the bsdf for the scattered direction is specular
    * returns true if scattering occures or false if it doesn't*/
    virtual bool scatter(const UBsdfSurfaceInfo& info, const glm::dvec3& w, glm::dvec3& scat_dirT, double& pPSA, glm::dvec3& bsdf_samplePSA, bool& specular) = 0;
};

#endif // UBSDF_H
