#include "ucamera.h"

UCamera::UCamera(double ratio, double verfovangle, const glm::dvec3& pos, const glm::dvec3& lookAt)
{
    m_camPos = pos;
    m_camLookAt = lookAt;
    m_camUp = glm::dvec3(0, 1, 0);

    m_verFovAngle = verfovangle;
    m_aspectRatio = ratio;
    m_horFovAngle = 2 * std::atan(m_aspectRatio*std::tan(m_verFovAngle / 2));

    m_view = glm::lookAtLH<double>(m_camPos, m_camLookAt, m_camUp);
}

void UCamera::setLook(const glm::dvec3& look) noexcept
{
    m_camLookAt = m_camPos + look;
    m_view = glm::lookAtLH<double>(m_camPos, m_camLookAt, m_camUp);
}

void UCamera::setLookAt(const glm::dvec3& lookAt) noexcept
{
    m_camLookAt = lookAt;
    m_view = glm::lookAtLH<double>(m_camPos, m_camLookAt, m_camUp);
}

void UCamera::setPos(const glm::dvec3& pos) noexcept
{
    m_camPos = pos;
    m_view = glm::lookAtLH<double>(m_camPos, m_camLookAt, m_camUp);
}

void UCamera::setHorizontalFOVAngle(double a) noexcept
{
    m_horFovAngle = a;
    m_verFovAngle = 2 * std::atan(std::tan(a / 2) / m_aspectRatio);
}

void UCamera::setVerticalFOVAngle(double a) noexcept
{
    m_verFovAngle = a;
    m_horFovAngle = 2 * std::atan(m_aspectRatio*std::tan(m_verFovAngle / 2));
}

void UCamera::setAspectRatio(double r) noexcept
{
    m_aspectRatio = r;
    m_horFovAngle = 2 * std::atan(m_aspectRatio*std::tan(m_verFovAngle / 2));
}

double UCamera::getHorizontalFOVAngle() const noexcept
{
    return m_horFovAngle;
}

double UCamera::getVerticalFOVAngle() const noexcept
{
    return m_verFovAngle;
}

double UCamera::getAspectRatio() const noexcept
{
    return m_aspectRatio;
}

double UCamera::getImagePlaneDistance() const noexcept
{
    return 1.0 / tanf(m_verFovAngle / 2.0);
}

const glm::dmat4x4& UCamera::getView() const noexcept
{
    return m_view;
}

const glm::dvec3& UCamera::getCamPosW() const noexcept
{
    return m_camPos;
}
