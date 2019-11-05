#ifndef UBSDFLAMBERTIAN_H
#define UBSDFLAMBERTIAN_H

#include <vector>
#include <utility>
#include "ugeometry.h"
#include "ubsdf.h"
#include "utexture.h"

class UBsdfLambertian : public UBsdf
{
public:
    UBsdfLambertian(std::shared_ptr<UTexture> texture, bool cosine_weighted);

    glm::dvec3 samplePSA(const UBsdfSurfaceInfo& info, const glm::dvec3& wiT, const glm::dvec3& woT) override;
    double pPSA(const UBsdfSurfaceInfo& info, const glm::dvec3& wsT, const glm::dvec3& wgT) override;
    bool scatter(const UBsdfSurfaceInfo& info, const glm::dvec3& w, glm::dvec3& scat_dirT, double& pPSA, glm::dvec3& bsdf_samplePSA, bool& specular) override;

private:
   bool m_cosine_weighted;
    std::shared_ptr<UTexture> m_texture;
};

#endif // UBSDFLAMBERTIAN_H
