#include "uscene.h"


void UScene::computeEmitterProbabilities()
{
	double total = 0;

	for(const auto& e : emitters())
	{
		glm::dvec3 P = e->power();
		total += (P.x + P.y + P.z) / e->area();
	}

	for(const auto& e : emitters())
	{
		glm::dvec3 P = e->power();
		e->setProbability((P.x + P.y + P.z) / (total * e->area()));
	}
}

bool UScene::visibility(const glm::dvec3& p0, const glm::dvec3& p1) noexcept
{
	double d;
	double points_distance = glm::length(p1 - p0);
	URay ray(p0, (p1 - p0) / points_distance);

	for(auto& o : objects())
	{
		if(o->intersects(ray, d))
		{
			if((d < points_distance) && (d > 0))
			{
				return false;
			}
		}
	}

	return true;
}

bool UScene::intersectionPoint(const URay& ray, USurfacePoint& closest_sp) noexcept
{
	USurfacePoint sp;
	double min_d = std::numeric_limits<double>::infinity();
	bool hit = false;
	double d;

	for(auto& o : objects())
	{
		if(o->intersectionPoint(ray, sp, d))
		{
			if(d < min_d)
			{
				min_d = d;
				closest_sp = sp;
				closest_sp.object = o;
				hit = true;
			}
		}
	}

	if(hit)
		return true;
	else
		return false;
}
