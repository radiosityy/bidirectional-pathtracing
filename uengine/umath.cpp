#include "umath.h"

URng& URng::get()
{
    static URng rng;

    return rng;
}

URng::URng()
{
    std::random_device rd;
    m_rand_generator = std::make_unique<std::mt19937>(rd());
    m_rand_distribution = std::make_unique<std::uniform_real_distribution<double>>(0.0, 1.0);
}

double URng::unitRand()
{
    return (*m_rand_distribution)(*m_rand_generator);
}

glm::dvec2 URng::sampleUnitRectStratified(size_t num_strata, size_t stratum_id)
{
    size_t num_divs = std::sqrt(num_strata);
    double d = 1.0 / static_cast<double>(num_divs);

    size_t x = stratum_id % num_divs;
    size_t y = stratum_id / num_divs;

    return glm::dvec2(
        d * (static_cast<double>(x) + unitRand()),
        d * (static_cast<double>(y) + unitRand())
    );
}

glm::dvec2 URng::sampleUnitDiskStratified(size_t num_strata, size_t stratum_id)
{
    glm::dvec2 uv = sampleUnitRectStratified(num_strata, stratum_id);

    double theta = 2 * M_PI * uv.x;
    double r = std::sqrt(uv.y);

    return glm::dvec2(r*std::cos(theta), r*std::sin(theta));
}

glm::dvec3 URng::samplePosHemUniform()
{
    double angle = unitRand() * M_PI * 2.0;
    double v = unitRand();
    double s = std::sqrt(1 - v*v);

    return glm::dvec3(std::cos(angle) * s, v, std::sin(angle) * s);
}

glm::dvec3 URng::samplePosHemCos()
{
    double angle = unitRand() * M_PI * 2.0;
    double s = unitRand();
    double y = std::sqrt(s);
    double r = std::sqrt(1 - s);

    return glm::dvec3(r*std::cos(angle), y, r*std::sin(angle));
}

glm::dvec3 URng::sampleUnitSphereUniform()
{
    double u = unitRand() * 2.0 * M_PI;
    double v = unitRand() * 2.0 - 1.0;
    double r = std::sqrt(1 - (v*v));

    return glm::dvec3(std::cos(u) * r, v, std::sin(u) * r);
}

glm::dvec3 URng::sampleTriangleUniform(const glm::dvec3& p0, const glm::dvec3& p1, const glm::dvec3& p2, double& u, double& v)
{
	double r1 = unitRand();
	double r2 = unitRand();

	double s = std::sqrt(r1);
	double m = s * r2;

	u = s - m;
	v = m;

	return (1.0 - s)*p0 + (s - m)*p1 + m*p2;
}


