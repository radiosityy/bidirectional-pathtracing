#ifndef MATERIAL_H
#define MATERIAL_H

#include <ubsdf.h>
#include <memory>

#include <ubsdflambertian.h>
#include <ubsdfperfectmirror.h>
#include <ubsdfdielectric.h>

class Material
{
public:
    virtual std::shared_ptr<UBsdf> bsdf() = 0;
};

class LatexPaint : public Material
{
public:
    LatexPaint(std::shared_ptr<UTexture> texture)
    {
        m_bsdf_lamb = std::make_shared<UBsdfLambertian>(texture, true);
    }

    std::shared_ptr<UBsdf> bsdf() override
    {
        if(URng::get().unitRand() < 0.8)
            return m_bsdf_lamb;
        else
            return nullptr;
    }

private:
    std::shared_ptr<UBsdfLambertian> m_bsdf_lamb;
};

class PerfectMirror : public Material
{
public:
    PerfectMirror(std::shared_ptr<UTexture> texture)
    {
        m_bsdf_mirror = std::make_shared<UBsdfPerfectMirror>(texture);
    }

    std::shared_ptr<UBsdf> bsdf() override
    {
        return m_bsdf_mirror;
    }

private:
    std::shared_ptr<UBsdfPerfectMirror> m_bsdf_mirror;
};

class Glossy : public Material
{
public:
    Glossy(std::shared_ptr<UTexture> texture, double diff, double mirr)
    {
        m_bsdf_mirror = std::make_shared<UBsdfPerfectMirror>(texture);
        m_bsdf_lambertian = std::make_shared<UBsdfLambertian>(texture,true);

        m_diff = diff;
        m_mirr = mirr;
    }

    std::shared_ptr<UBsdf> bsdf() override
    {
        auto r = URng::get().unitRand();

        if(r < m_diff)
            return m_bsdf_lambertian;

        r -= m_diff;

        if(r < m_mirr)
            return m_bsdf_mirror;

        return nullptr;
    }

private:
    double m_diff;
    double m_mirr;

    std::shared_ptr<UBsdfPerfectMirror> m_bsdf_mirror;
    std::shared_ptr<UBsdfLambertian> m_bsdf_lambertian;
};

class Dielectric : public Material
{
public:
    Dielectric(std::shared_ptr<UTexture> texture, double eta)
    {
        m_bsdf_dielectric = std::make_shared<UBsdfDielectric>(texture, eta);
    }

    std::shared_ptr<UBsdf> bsdf() override
    {
        return m_bsdf_dielectric;
    }

private:
    std::shared_ptr<UBsdfDielectric> m_bsdf_dielectric;
};

#endif // MATERIAL_H
