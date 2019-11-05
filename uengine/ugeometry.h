#ifndef UGEOMETRY_H
#define UGEOMETRY_H

#include "umath.h"

double triangleArea(const glm::dvec3&, const glm::dvec3&, const glm::dvec3&);
glm::dvec3 triangleUniformSample(const glm::dvec3&, const glm::dvec3&, const glm::dvec3&, double& u, double& v);

glm::dvec3 transformPoint(const glm::dmat4x4 T, const glm::dvec3 P);
glm::dvec3 transformPointT(const glm::dmat4x4 T, const glm::dvec3 P);
glm::dvec3 transformVector(const glm::dmat4x4 T, const glm::dvec3 V, bool normalize = true);
glm::dvec3 transformVectorT(const glm::dmat4x4 T, const glm::dvec3 V, bool normalize = true);

class URay
{
public:
    URay() = default;
    URay(glm::dvec3 origin, glm::dvec3 dir);

    void setDir(glm::dvec3) noexcept;
    void setOrigin(glm::dvec3) noexcept;

    glm::dvec3 origin() const noexcept;
    glm::dvec4 origniH() const noexcept;
    glm::dvec3 dir() const noexcept;
    glm::dvec4 dirH() const noexcept;

    URay transform(const glm::dmat4x4&) const noexcept;
    void transformDir(const glm::dmat4x4&) noexcept;
    void transformOrigin(const glm::dmat4x4&) noexcept;

    bool intersectUnitSphere(double& d) const noexcept;
    bool intersectTriangle(const glm::dvec3& p0, const glm::dvec3& p1, const glm::dvec3& p2, double& d, double& u, double& v) const noexcept;

private:
    glm::dvec4 m_origin;
    glm::dvec4 m_dir;
};

#endif // UGEOMETRY_H
