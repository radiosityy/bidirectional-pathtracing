#ifndef UENGINE_H
#define UENGINE_H

/*if in debug mode define application's own debug macro*/
#ifndef NDEBUG
#define UDEBUG
#endif

#include "uscene.h"
#include "urenderer.h"
#include "uutils.h"
#include "ugeometry.h"
#include "uconverter.h"

#include <functional>
#include <string>

enum class UResult{USuccess, UUninitialized, UInvalidScene, UInvalidFormat, UStopped, UNoData, UError};
enum class URendererType{BDPT};

class UEngine
{
public:
    static UEngine& get() noexcept;

    /*explicitly disable copy and move operations*/
    UEngine (const UEngine&) = delete;
    UEngine& operator=(const UEngine&) = delete;
    UEngine (UEngine&&) = delete;
    UEngine& operator=(UEngine&&) = delete;

    UResult newRendering(const URenderParameters&, URendererType) noexcept;
    UResult saveRendering(const std::string& filename) noexcept;
    UResult loadRendering(const std::string& filename, URenderParameters&, URendererType&, size_t& curr_pass) noexcept;
    UResult imageRGB(std::vector<glm::dvec3>& img_data, URgbFormat, double gamma, size_t& img_width, size_t& img_height) noexcept;

    void setScene(const std::shared_ptr<UScene>&) noexcept;

    UResult renderPass(size_t num_threads, std::function<void(double)> update_progress_callback = nullptr);
    /*stops current rendering*/
    void stop();
	
private:
	UEngine() = default;

	/*init buffers to store computed values for each pixel*/
	bool initPixelBuffers(size_t res_x, size_t res_y);

	std::shared_ptr<UBuffer2D<glm::dvec3>> m_pixel_buffers[2];
	size_t m_pixel_buffer_write;
	size_t m_pixel_buffer_read;

    /*---rendering---*/
    URendererType m_renderer_type;
    URenderParameters m_render_params;
    size_t m_curr_pass;

	/*---supported parameters---*/
	const size_t m_max_threads = 64;

    std::shared_ptr<UScene> m_scene;
    std::unique_ptr<URenderer> m_renderer;
};

#endif //UENGINE_H
