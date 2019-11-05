#ifndef UBSDFDIELECTRIC_H
#define UBSDFDIELECTRIC_H

#include "ubsdf.h"
#include "utexture.h"

class UBsdfDielectric : public UBsdf
{
public:
    UBsdfDielectric(std::shared_ptr<UTexture> texture, double eta);

    glm::dvec3 samplePSA(const UBsdfSurfaceInfo& info, const glm::dvec3& wiT, const glm::dvec3& woT) override;
    double pPSA(const UBsdfSurfaceInfo& info, const glm::dvec3& wsT, const glm::dvec3& wgT) override;
    bool scatter(const UBsdfSurfaceInfo& info, const glm::dvec3& w, glm::dvec3& scat_dirT, double& pPSA, glm::dvec3& bsdf_samplePSA, bool& specular) override;

private:
    double m_eta;
    std::shared_ptr<UTexture> m_texture;
};

#endif // UBSDFDIELECTRIC_H
