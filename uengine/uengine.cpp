#include "uengine.h"
#include "ubdptrenderer.h"

#include <thread>
#include <fstream>
#include <iostream>

UEngine& UEngine::get() noexcept
{
	static UEngine engine;
	
	return engine;
}

void UEngine::setScene(const std::shared_ptr<UScene>& scene) noexcept
{
	m_scene = std::shared_ptr<UScene>(scene);

	m_scene->computeEmitterProbabilities();
}

bool UEngine::initPixelBuffers(size_t res_x, size_t res_y)
{
	m_pixel_buffers[0] = std::make_unique<UBuffer2D<glm::dvec3>>(res_x, res_y);
	m_pixel_buffers[1] = std::make_unique<UBuffer2D<glm::dvec3>>(res_x, res_y);

	if((m_pixel_buffers[0] == nullptr) || (m_pixel_buffers[1] == nullptr))
		return false;

	return true;
}

UResult UEngine::newRendering(const URenderParameters& params, URendererType renderer_type) noexcept
{
	if(m_scene == nullptr)
	{
		return UResult::UInvalidScene;
	}

	m_curr_pass = 0;
	m_render_params = params;
	m_pixel_buffer_write = 0;
	m_pixel_buffer_read = 1;

	if(!initPixelBuffers(m_render_params.img_res_x, m_render_params.img_res_y))
		return UResult::UError;

	switch(renderer_type)
	{
	case URendererType::BDPT:
		m_renderer = std::make_unique<UBDPTRenderer>();
		break;
	}

	m_renderer_type = renderer_type;

	if(!m_renderer->initialize(params, m_scene))
	{
		return UResult::UError;
	}

	return UResult::USuccess;
}

UResult UEngine::saveRendering(const std::string& filename) noexcept
{
	std::ofstream out;

	out.open(filename, std::ios::binary);
	if(!out.is_open())
	{
		return UResult::UError;
	}

	/*save the rendering parameters*/
	out.write(reinterpret_cast<const char*>(&m_curr_pass), sizeof(m_curr_pass));
	out.write(reinterpret_cast<const char*>(&m_render_params), sizeof(m_render_params));
	out.write(reinterpret_cast<const char*>(&m_renderer_type), sizeof(m_renderer_type));

	/*save the pixel buffer*/
	out.write(reinterpret_cast<char*>(m_pixel_buffers[m_pixel_buffer_read]->ptr()), sizeof(glm::dvec3) * m_render_params.img_res_x * m_render_params.img_res_y);

	out.close();

	return UResult::USuccess;
}

UResult UEngine::loadRendering(const std::string& filename, URenderParameters& params, URendererType& rt, size_t& curr_pass) noexcept
{
	if(m_scene == nullptr)
	{
		return UResult::UInvalidScene;
	}

	std::ifstream in;
	in.open(filename, std::ios::binary);

	if(!in.is_open())
	{
		return UResult::UError;
	}

	m_pixel_buffer_write = 0;
	m_pixel_buffer_read = 1;

	/*load the rendering parameters*/
	in.read(reinterpret_cast<char*>(&m_curr_pass), sizeof(m_curr_pass));
	in.read(reinterpret_cast<char*>(&m_render_params), sizeof(m_render_params));
	in.read(reinterpret_cast<char*>(&m_renderer_type), sizeof(m_renderer_type));

	/*load the pixel buffer*/
	if(initPixelBuffers(m_render_params.img_res_x, m_render_params.img_res_y))
	{
		in.read(reinterpret_cast<char*>(m_pixel_buffers[m_pixel_buffer_read]->ptr()), sizeof(glm::dvec3) * m_render_params.img_res_x * m_render_params.img_res_y);
		in.close();
	}
	else
	{
		in.close();
		return UResult::UError;
	}

	switch(m_renderer_type)
	{
	case URendererType::BDPT:
		m_renderer = std::make_unique<UBDPTRenderer>();
		break;
	default:
		return UResult::UInvalidFormat;
	}

	if(!m_renderer->initialize(m_render_params, m_scene))
	{
		return UResult::UError;
	}

	params = m_render_params;
	curr_pass = m_curr_pass;
	rt = m_renderer_type;

	return UResult::USuccess;
}

UResult UEngine::renderPass(size_t num_threads, std::function<void(double)> update_progress_callback)
{
	/*check if a renderer is set*/
	if(m_renderer == nullptr)
	{
		return UResult::UUninitialized;
	}

	/*if it's not the first pass, set the write buffer to values calculated during previous passes*/
	if(m_curr_pass != 0)
	{
		m_pixel_buffers[m_pixel_buffer_write]->setFrom(*m_pixel_buffers[m_pixel_buffer_read]);
	}

	/*choose the number of threads to use*/
	if(num_threads <= 0)
		num_threads = 1;
	else if (num_threads > m_max_threads)
		num_threads = m_max_threads;

	bool complete = m_renderer->renderPass(m_pixel_buffers[m_pixel_buffer_write], m_curr_pass, num_threads, update_progress_callback);

	/*if pass successfully completed*/
	if(complete)
	{
		/*increment pass counter*/
		m_curr_pass++;
		/*swap pixel buffers*/
		std::swap(m_pixel_buffer_read, m_pixel_buffer_write);

		return UResult::USuccess;
	}
	else
	{
		return UResult::UStopped;
	}
}

UResult UEngine::imageRGB(std::vector<glm::dvec3>& img_data, URgbFormat format, double gamma, size_t& img_width, size_t& img_height) noexcept
{
	if((m_curr_pass == 0) || (gamma <= 0) || (m_pixel_buffers[m_pixel_buffer_read] == nullptr))
		return UResult::UNoData;

	img_width = m_render_params.img_res_x;
	img_height = m_render_params.img_res_y;

	img_data.resize(img_width * img_height);

	for(size_t p = 0; p < img_width * img_height; p++)
	{
		size_t px = p % img_width;
		size_t py = p / img_width;

		glm::dvec3 radiance = m_pixel_buffers[m_pixel_buffer_read]->at(px, py) / static_cast<double>(m_curr_pass);
		img_data[p] = UConverter::radianceToRGB(radiance, format, gamma);
	}

	return UResult::USuccess;
}

void UEngine::stop()
{
	if(m_renderer != nullptr)
		m_renderer->stop();
}
