#include "ubsdfdielectric.h"

double fersnel(double eta_i, double eta_t, double cos_i, double cos_t)
{
    double rp = (eta_t * cos_i - eta_i * cos_t) / (eta_t * cos_i + eta_i * cos_t);
    double rs = (eta_i * cos_i - eta_t * cos_t) / (eta_i * cos_i + eta_t * cos_t);

    return 0.5*(rp*rp + rs*rs);
}

UBsdfDielectric::UBsdfDielectric(std::shared_ptr<UTexture> texture, double eta)
{
    m_texture = texture;
    m_eta = eta;
}

glm::dvec3 UBsdfDielectric::samplePSA(const UBsdfSurfaceInfo& info, const glm::dvec3& wiT, const glm::dvec3& woT)
{
    glm::dmat3x3 TNB;
    TNB[0] = info.Ts;
    TNB[1] = info.Ns;
    TNB[2] = info.Bs;

//    glm::dvec3 wT = glm::normalize(wiT * TNB);

    double n, nt;
    glm::dvec3 N;

    if(wiT.y < 0)
    {
        N = glm::dvec3(0.0, -1.0, 0.0);
        n = m_eta;
        nt = info.eta_t;
    }
    else
    {
        N = glm::dvec3(0.0, 1.0, 0.0);
        n = info.eta_t;
        nt = m_eta;
    }

    double c = ((n*n) / (nt*nt)) * (1.0 - std::pow(wiT.y, 2));

    if(c > 1.0)
    {

        if(wiT.y * woT.y <= 0)
            return {0, 0, 0};
        else
            return m_texture->sample(info.tex_u, info.tex_v);
    }
    else
    {
        double R = fersnel(n, nt, N.y * wiT.y, std::sqrt(1.0 - c));
        double T = 1.0 - R;

        if(wiT.y * woT.y <= 0)
            return T * m_texture->sample(info.tex_u, info.tex_v);
        else
            return R * m_texture->sample(info.tex_u, info.tex_v);
    }
}

double UBsdfDielectric::pPSA(const UBsdfSurfaceInfo& info, const glm::dvec3& wsT, const glm::dvec3& wgT)
{
    glm::dmat3x3 TNB;
    TNB[0] = info.Ts;
    TNB[1] = info.Ns;
    TNB[2] = info.Bs;

//    glm::dvec3 wT = glm::normalize(wgT * TNB);

    double n, nt;
    glm::dvec3 N;

    if(wgT.y < 0)
    {
        N = glm::dvec3(0.0, -1.0, 0.0);
        n = m_eta;
        nt = info.eta_t;
    }
    else
    {
        N = glm::dvec3(0.0, 1.0, 0.0);
        n = info.eta_t;
        nt = m_eta;
    }

    double c = ((n*n) / (nt*nt)) * (1.0 - std::pow(wgT.y, 2));

    if(c > 1.0)
    {
        if(wsT.y * wgT.y <= 0)
            return 0.0;
        else
            return 1.0;
    }
    else
    {
        double R = fersnel(n, nt, N.y * wgT.y, std::sqrt(1.0 - c));
        double T = 1.0 - R;

        if(wsT.y * wgT.y <= 0)
            return T;
        else
            return R;
    }
}

bool UBsdfDielectric::scatter(const UBsdfSurfaceInfo& info, const glm::dvec3& w, glm::dvec3& scat_dirT, double& pPSA, glm::dvec3& bsdf_samplePSA, bool& specular)
{
    if(glm::dot(w, info.Ns) * glm::dot(w, info.Ng) <= 0)
        return false;

    specular = true;

    glm::dmat3x3 TNB;
    TNB[0] = info.Ts;
    TNB[1] = info.Ns;
    TNB[2] = info.Bs;

    glm::dvec3 wT = glm::normalize(w * TNB);

    double n, nt;
    glm::dvec3 N;

    if(wT.y < 0)
    {
        N = glm::dvec3(0.0, -1.0, 0.0);
        n = m_eta;
        nt = info.eta_t;
    }
    else
    {
        N = glm::dvec3(0.0, 1.0, 0.0);
        n = info.eta_t;
        nt = m_eta;
    }

    double eta_r = n/nt; //eta ratio

    double c = (eta_r * eta_r) * (1.0 - std::pow(wT.y, 2));
    double R, T, c1;

    if(c > 1.0)
    {
        R = 1.0;
    }
    else
    {
        c1 = std::sqrt(1.0 - c);
        R = fersnel(n, nt, N.y * wT.y, c1);
    }

    T = 1.0 - R;

    /*reflection*/
    if(URng::get().unitRand() < R)
    {
        scat_dirT = glm::normalize(glm::reflect(-wT, N));

        pPSA = R;
        bsdf_samplePSA = R * m_texture->sample(info.tex_u, info.tex_v);
    }
    else /*refraction*/
    {
        scat_dirT = glm::normalize(eta_r * (-wT) - N*(eta_r*glm::dot(N, -wT) + c1));

        pPSA = T;
        bsdf_samplePSA = T * m_texture->sample(info.tex_u, info.tex_v);
    }

    return true;
}

