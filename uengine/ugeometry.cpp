#include "ugeometry.h"
#include "uengine.h"

double triangleArea(const glm::dvec3& p0, const glm::dvec3& p1, const glm::dvec3& p2)
{
    glm::dvec3 AB = p1 - p0;
    glm::dvec3 AC = p2 - p0;

    double l1 = glm::length(AB);
    double l2 = glm::length(AC);
    double c = glm::dot(AB / l1, AC / l2);
    double s = std::sqrt(1 - c*c);

    return 0.5 * l1 * l2 * s;
}

/*------------------------------------ray------------------------------------*/

URay::URay(glm::dvec3 origin, glm::dvec3 dir)
{
	m_origin = glm::dvec4(origin, 1);
	m_dir = glm::dvec4(dir, 0);
}

void URay::setDir(glm::dvec3 dir) noexcept
{
	m_dir = glm::dvec4(dir, 0);
}

void URay::setOrigin(glm::dvec3 origin) noexcept
{
	m_origin = glm::dvec4(origin, 1);
}

glm::dvec3 URay::origin() const noexcept
{
	return glm::dvec3(m_origin);
}

glm::dvec4 URay::origniH() const noexcept
{
	return m_origin;
}

glm::dvec3 URay::dir() const noexcept
{
	return glm::dvec3(m_dir);
}

glm::dvec4 URay::dirH() const noexcept
{
	return m_dir;
}

URay URay::transform(const glm::dmat4x4& T) const noexcept
{
	return URay(glm::dvec3(T*m_origin), glm::dvec3(T*m_dir));
}

void URay::transformDir(const glm::dmat4x4& T) noexcept
{
	m_dir = T*m_dir;
}

void URay::transformOrigin(const glm::dmat4x4& T) noexcept
{
	m_origin = T*m_origin;
}

bool URay::intersectUnitSphere(double& d) const noexcept
{
    double a = glm::dot(dir(), dir());
    double b = 2 * glm::dot(origin(), dir());
    double c = glm::dot(origin(), origin()) - 1;

    double delta = (b*b) - (4*a*c);

    if(delta < 0)
        return false;

    double sd = std::sqrt(delta);

    d = (-b - sd) / (2*a);

    if(d > 0)
        return true;

    d = (-b + sd) / (2*a);

    if(d < 0)
        return false;

    return true;
}

bool URay::intersectTriangle(const glm::dvec3& p0, const glm::dvec3& p1, const glm::dvec3& p2, double& d, double& u, double& v) const noexcept
{
	glm::dvec3 e1 = p1 - p0;
	glm::dvec3 e2 = p2 - p0;
	glm::dvec3 m = origin() - p0;

	glm::dvec3 c1 = glm::cross(dir(), e2);
	glm::dvec3 c2 = glm::cross(m, e1);
	double a = glm::dot(e1, c1);

	d = glm::dot(e2, c2) / a;
	u = glm::dot(m, c1) / a;
	v = glm::dot(dir(), c2) / a;

	if(d > 0 && u >= 0 && v >= 0 && u+v <= 1)
		return true;
	else
		return false;
}

glm::dvec3 transformPoint(const glm::dmat4x4 T, const glm::dvec3 P)
{
	return glm::dvec3(T * glm::dvec4(P, 1.0));
}

glm::dvec3 transformPointT(const glm::dmat4x4 T, const glm::dvec3 P)
{
	return glm::dvec3(glm::dvec4(P, 1.0) * T);
}

glm::dvec3 transformVector(const glm::dmat4x4 T, const glm::dvec3 V, bool normalize)
{
	glm::dvec3 res = glm::dvec3(T * glm::dvec4(V, 0.0));
	return normalize ? glm::normalize(res) : res;
}

glm::dvec3 transformVectorT(const glm::dmat4x4 T, const glm::dvec3 V, bool normalize)
{
	glm::dvec3 res = glm::dvec3(glm::dvec4(V, 0.0) * T);
	return normalize ? glm::normalize(res) : res;
}
