#ifndef UUTILS_H
#define UUTILS_H

#include "umath.h"
#include "ubsdf.h"

#include <cstring>

class UObject;

struct URenderParameters
{
	size_t img_res_x;
	size_t img_res_y;
	size_t pixel_subdiv;
	size_t lens_subdiv;
	size_t min_depth;
	double focus_plane_distance;
	double lens_size;
};

struct USurfacePoint
{
	glm::dmat4x4 W;
	glm::dmat4x4 invW;
	std::shared_ptr<UObject> object;
	std::shared_ptr<UBsdf> bsdf;
	glm::dvec3 pos;
	glm::dvec3 Ng; //geometric normal
	glm::dvec3 Ns; //shading normal
	glm::dvec3 Ts; //shading tangent
	glm::dvec3 Bs; //shading bitangent
	double tex_u;
	double tex_v;
};

template<class T>
class UBuffer2D
{
public:
	UBuffer2D(size_t size_x, size_t size_y);
	~UBuffer2D();

	T& at(size_t x, size_t y);
	void setFrom(const UBuffer2D&);
	void* ptr();

private:
	T** buf;

	size_t m_size_x;
	size_t m_size_y;
};

template<class T>
UBuffer2D<T>::UBuffer2D(size_t size_x, size_t size_y)
{
	m_size_x = size_x;
	m_size_y = size_y;

	buf = new T*[size_y];
	buf[0] = new T[size_x * size_y];

	for(size_t y = 1; y < size_y; y++)
	{
		buf[y] = buf[0] + y * size_x;
	}

	std::memset(buf[0], 0, sizeof(T) * size_x * size_y);
}

template<class T>
UBuffer2D<T>::~UBuffer2D()
{
	if(buf != nullptr)
	{
		delete[] buf[0];
		delete[] buf;
	}
}

template<class T>
T& UBuffer2D<T>::at(size_t x, size_t y)
{
	return buf[y][x];
}

template<class T>
void* UBuffer2D<T>::ptr()
{
	return buf[0];
}

template<class T>
void UBuffer2D<T>::setFrom(const UBuffer2D& src)
{
	std::memcpy(buf[0], src.buf[0], sizeof(T) * m_size_x * m_size_y);
}

#endif //UUTILS_H
