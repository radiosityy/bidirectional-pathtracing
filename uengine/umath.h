#ifndef UMATH_H
#define UMATH_H

#define GLM_FORCE_INLINE
#define GLM_FORCE_SSE4

#include <glm/fwd.hpp>
#include <glm/glm.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <random>
#include <memory>

class URng
{
public:
	static URng& get();

	double unitRand();
	glm::dvec2 sampleUnitRectStratified(size_t num_strata, size_t stratum_id);
	glm::dvec2 sampleUnitDiskStratified(size_t num_strata, size_t stratum_id);
	glm::dvec3 samplePosHemUniform();
	glm::dvec3 samplePosHemCos();
	glm::dvec3 sampleUnitSphereUniform();
	glm::dvec3 sampleTriangleUniform(const glm::dvec3& p0, const glm::dvec3& p1, const glm::dvec3& p2, double& u, double& v);
private:
	URng();

	std::unique_ptr<std::mt19937> m_rand_generator;
	std::unique_ptr<std::uniform_real_distribution<double>> m_rand_distribution;
};

#endif //UMATH_H
