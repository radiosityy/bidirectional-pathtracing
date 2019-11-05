#ifndef USCENE_H
#define USCENE_H

#include <vector>
#include <memory>

#include "uobject.h"
#include "uemitter.h"
#include "ucamera.h"

struct USurfacePoint;

class UScene
{
public:
    virtual UCamera& camera() = 0;
    virtual const std::vector<std::shared_ptr<UObject>>& objects() = 0;
    virtual const std::vector<std::shared_ptr<UEmitter>>& emitters() = 0;

	void computeEmitterProbabilities();
	/*returns whether two points are directly visible from one another in the current scene*/
	bool visibility(const glm::dvec3& p0, const glm::dvec3& p1) noexcept;
	/*iterates over objects in the current scene, finds closest intersection along the ray
	* and returns the intersection point in a UIntersectionPoint structure*/
	bool intersectionPoint(const URay&, USurfacePoint&) noexcept;
};

#endif // USCENE_H
