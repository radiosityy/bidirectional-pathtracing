#ifndef UBDPTRENDERER_H
#define UBDPTRENDERER_H

#include "urenderer.h"

#include <mutex>
#include <atomic>

struct UPathVertex
{
	USurfacePoint sp;
	/*accumulated "weight"  f(x) / p(x)  at this vertex*/
	glm::dvec3 a;
	/*flag indicating whether this vertex is specular*/
	bool specular;
	/* Propability densities for generating this vertex as a part of the light/eye subpath.
	 * These probabilities are with respect to area measure, so they can by computed as
	 * the probabilities with respect to projected solid angle measure multiplied by the G factor.
	 * For the vertices at the emitter/lens surface the "light"/"eye" densities are just the densities for
	 * generating the vertex at the surface which is directly computed with respect to area measure. For the last
	 * vertex in the light subpath, the "eye" density is ignored. Similarly the "light" density in the eye subpath*/
	double p_light_A;
	double p_eye_A;
};

class UBDPTRenderer : public URenderer
{
public:
    virtual bool initialize(const URenderParameters&, std::shared_ptr<UScene>) override;
    virtual bool renderPass(std::shared_ptr<UBuffer2D<glm::dvec3>> pixel_buffer, size_t curr_pass, size_t num_threads, std::function<void(double)>&) override;
    virtual void stop() override;

private:
	void renderPixel(size_t px, size_t py);

	glm::dvec3 computeEyeSubpath(std::vector<UPathVertex>& subpath, size_t px, size_t py, size_t lens_sample_id, size_t pixel_sample_id);
	void computeLightSubpath(std::vector<UPathVertex>& subpath);

	bool connectionFactor(const std::vector<UPathVertex>& light_subpath, const std::vector<UPathVertex>& eye_subpath, size_t s, size_t t, glm::dvec3& c);

	glm::dvec3 s0sample(const std::vector<UPathVertex>& subpath, const UPathVertex& emitter_vertex);
	/*compute the p_s+1 probability*/
	double p_sp1(const std::vector<UPathVertex>& light_subpath, const std::vector<UPathVertex>& eye_subpath, size_t s, size_t t);
	/*compute the p_s-1 probability*/
	double p_sm1(const std::vector<UPathVertex>& light_subpath, const std::vector<UPathVertex>& eye_subpath, size_t s, size_t t);
	double weight(const std::vector<UPathVertex>& light_subpath, const std::vector<UPathVertex>& eye_subpath, size_t s, size_t t);

	std::mutex m_pixel_buffer_mutex;
	std::shared_ptr<UBuffer2D<glm::dvec3>> m_pixel_buffer;

	size_t m_num_renderred_pixels;
	std::mutex m_update_progress_mutex;

    /*---render parameters---*/
    size_t m_img_res_x;
    size_t m_img_res_y;
    size_t m_num_pixel_strata;
    size_t m_num_lens_strata;
    double m_focus_plane_distance;
    double m_lens_radius;
    size_t m_min_depth;
    size_t m_curr_pass;

    /*---perspective---*/
    double m_image_plane_distance;
    double m_image_plane_ratio;
    double m_image_plane_area;
    double m_pixel_width;
    double m_pixel_height;
    double m_pixel_area;
    double m_lens_area;
    double m_lens_stratum_area;
    double m_pixel_stratum_area;
    glm::dmat4x4 m_V;
    glm::dmat4x4 m_invV;

    std::shared_ptr<UScene> m_scene;

    const double m_W = 1.0; //total emitted importance

	std::atomic<bool> m_stop;
};

#endif // UBDPTRENDERER_H
