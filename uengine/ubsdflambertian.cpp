#include "ubsdflambertian.h"
#include "uengine.h"

UBsdfLambertian::UBsdfLambertian(std::shared_ptr<UTexture> texture, bool cosine_weighted)
{
    m_cosine_weighted = cosine_weighted;
    m_texture = texture;
}

glm::dvec3 UBsdfLambertian::samplePSA(const UBsdfSurfaceInfo& info, const glm::dvec3& wiT, const glm::dvec3& woT)
{
    glm::dmat3x3 TNB;
    TNB[0] = info.Ts;
    TNB[1] = info.Ns;
    TNB[2] = info.Bs;

    glm::dvec3 wiL = glm::normalize(TNB * wiT);
    glm::dvec3 woL = glm::normalize(TNB * woT);

    if(glm::dot(info.Ng, wiL) * glm::dot(info.Ng, woL) <= 0)
        return {0, 0, 0};
    if(wiT.y * woT.y <= 0)
        return {0, 0, 0};
    else
        return (1.0 / M_PI) * m_texture->sample(info.tex_u, info.tex_v);
}

double UBsdfLambertian::pPSA(const UBsdfSurfaceInfo& info, const glm::dvec3& wsT, const glm::dvec3& wgT)
{
    glm::dmat3x3 TNB;
    TNB[0] = info.Ts;
    TNB[1] = info.Ns;
    TNB[2] = info.Bs;

    glm::dvec3 wsL = glm::normalize(TNB * wsT);
    glm::dvec3 wgL = glm::normalize(TNB * wgT);

    if(glm::dot(info.Ng, wsL) * glm::dot(info.Ng, wgL) <= 0)
        return 0;
    if(wsT.y * wgT.y <= 0)
        return 0;
    else
    {
        if(m_cosine_weighted)
            return (1.0 / M_PI);
        else
            return (1.0 / (2.0 * M_PI * std::abs(wsT.y)));
    }
}

bool UBsdfLambertian::scatter(const UBsdfSurfaceInfo& info, const glm::dvec3& w, glm::dvec3& scat_dirT, double& pPSA, glm::dvec3& bsdf_samplePSA, bool& specular)
{
    glm::dmat3x3 TNB;
    TNB[0] = info.Ts;
    TNB[1] = info.Ns;
    TNB[2] = info.Bs;

    if(glm::dot(w, info.Ns) * glm::dot(w, info.Ng) <= 0)
        return false;

    glm::dvec3 wT = glm::normalize(w * TNB);

    if(m_cosine_weighted)
    {
        scat_dirT = URng::get().samplePosHemCos();
        pPSA = 1.0 / M_PI;
    }
    else
    {
        scat_dirT = URng::get().samplePosHemUniform();
        pPSA = (1.0 / (2.0 * M_PI * std::abs(scat_dirT.y)));
    }

    if(wT.y < 0)
        scat_dirT *= -1.0;

    specular = false;
    bsdf_samplePSA = (1.0 / M_PI) * m_texture->sample(info.tex_u, info.tex_v);

    return true;
}
