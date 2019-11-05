#ifndef UCAMERA_H
#define UCAMERA_H

#include "umath.h"
#include <utility>

class UCamera
{
public:
    UCamera(double ratio, double verfovangle, const glm::dvec3& pos = {0, 0, 0}, const glm::dvec3& lookAt = {0, 0, 1});
    ~UCamera() = default;

    /*explicitly disable copy and move operations*/
    UCamera (const UCamera&) = delete;
    UCamera& operator=(const UCamera&) = delete;
    UCamera (UCamera&&) = delete;
    UCamera& operator=(UCamera&&) = delete;

    /*sets camera facing direction relative to its position*/
    void setLook(const glm::dvec3&) noexcept;
    /*set point for camera to look at*/
    void setLookAt(const glm::dvec3&) noexcept;
    /*sets camera position*/
    void setPos(const glm::dvec3&) noexcept;
    /*set camera's horizaontal and vertical FOV angles*/
    void setHorizontalFOVAngle(double) noexcept;
    void setVerticalFOVAngle(double) noexcept;
    /*sets aspect ratio (width/height)*/
    void setAspectRatio(double) noexcept;

    double getHorizontalFOVAngle() const noexcept;
    double getVerticalFOVAngle() const noexcept;
    double getAspectRatio() const noexcept;
    double getImagePlaneDistance() const noexcept;

    /*returns the view matrix*/
    const glm::dmat4x4& getView() const noexcept;
    /*returns camera position in world coordinates*/
    const glm::dvec3& getCamPosW() const noexcept;

private:
    /*view matrix*/
    glm::dmat4x4 m_view;

    /*camera position*/
    glm::dvec3 m_camPos;
    glm::dvec3 m_camLookAt;
    glm::dvec3 m_camUp;

    /*FOV angles*/
    double m_verFovAngle;
    double m_horFovAngle;
    /*aspect ratio (width/height)*/
    double m_aspectRatio;
};

#endif //UCAMERA_H
