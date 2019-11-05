#ifndef URENDERER_H
#define URENDERER_H

#include "uutils.h"

#include <functional>

class UScene;

class URenderer
{
public:
	virtual bool initialize(const URenderParameters&, std::shared_ptr<UScene>) = 0;
	virtual bool renderPass(std::shared_ptr<UBuffer2D<glm::dvec3>>, size_t curr_pass, size_t num_threads, std::function<void(double)>&) = 0;
	virtual void stop() = 0;
};

#endif // URENDERER_H
