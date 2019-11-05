#include "ubdptrenderer.h"

#include "uscene.h"

#include <thread>

bool UBDPTRenderer::initialize(const URenderParameters& params, std::shared_ptr<UScene> scene)
{
	m_scene = scene;

	m_image_plane_ratio = m_scene->camera().getAspectRatio();
	m_image_plane_distance = m_scene->camera().getImagePlaneDistance();
	m_V = m_scene->camera().getView();
	m_invV = glm::inverse(m_scene->camera().getView());

	m_img_res_x = params.img_res_x;
	m_img_res_y = params.img_res_y;
	m_num_pixel_strata = params.pixel_subdiv * params.pixel_subdiv;
	m_num_lens_strata = params.lens_subdiv * params.lens_subdiv;
	m_focus_plane_distance = params.focus_plane_distance;
	m_lens_radius = params.lens_size;
	m_min_depth = params.min_depth;

	m_image_plane_area = 4 * m_image_plane_ratio;
	m_pixel_area = m_image_plane_area / static_cast<double>(m_img_res_x * m_img_res_y);
	m_pixel_width = 2.0 * m_image_plane_ratio / static_cast<double>(m_img_res_x);
	m_pixel_height = 2.0 / static_cast<double>(m_img_res_y);
	m_pixel_stratum_area = (m_pixel_area) / static_cast<double>(m_num_pixel_strata);
	m_lens_stratum_area = M_PI * m_lens_radius * m_lens_radius;
	m_lens_stratum_area = m_lens_area / static_cast<double>(m_num_lens_strata);

	return true;
}

bool UBDPTRenderer::renderPass(std::shared_ptr<UBuffer2D<glm::dvec3> > pixel_buffer, size_t curr_pass, size_t num_threads, std::function<void(double)>& update_progress)
{
	m_stop = false;

	m_curr_pass = curr_pass;
	m_pixel_buffer = pixel_buffer;
	m_num_renderred_pixels = 0;

	std::vector<std::unique_ptr<std::thread>> threads(num_threads);

	auto fun = [&](size_t id){
		for(size_t px = 0; px < m_img_res_x/num_threads; px++)
			for(size_t py = 0; py < m_img_res_y; py++)
			{
				if(m_stop)
					return;

				renderPixel(px + id*m_img_res_x/num_threads, py);

				if(update_progress != nullptr)
				{
					std::lock_guard<std::mutex> lock(m_update_progress_mutex);
					m_num_renderred_pixels++;

					double progress = static_cast<double>(m_num_renderred_pixels) / static_cast<double>(m_img_res_x * m_img_res_y);

					update_progress(progress);
				}
			}
	};

	for(size_t t = 0; t < num_threads; t++)
	{
		threads[t] = std::make_unique<std::thread>(fun, t);
	}

	for(auto& t : threads)
	{
		if(t->joinable())
			t->join();
	}

	if(m_stop)
		return false;
	else
		return true;
}

void UBDPTRenderer::stop()
{
	m_stop = true;
}

void UBDPTRenderer::renderPixel(size_t px, size_t py)
{
	/*the total measurement for the pixel*/
	glm::dvec3 I = glm::dvec3(0, 0, 0);
	std::vector<UPathVertex> light_subpath;
	std::vector<UPathVertex> eye_subpath;

	size_t pixel_sample = m_curr_pass % m_num_pixel_strata;
	size_t lens_sample = m_curr_pass % m_num_lens_strata;

	I += computeEyeSubpath(eye_subpath, px, py, lens_sample, pixel_sample);
	computeLightSubpath(light_subpath);

	/* we don't consider path's where t=0 at all; paths s=0 are sampled while computing the eye subpath;
	 * here we only consider paths where s,t > 0*/
	if((light_subpath.size() > 0) && (eye_subpath.size() > 0))
		for(size_t s = 1; s <= light_subpath.size(); s++)
		{
			for(size_t t = 1; t <= eye_subpath.size(); t++)
			{
				//TODO: s==1 as a special case

				size_t t1_pixel_x, t1_pixel_y;
				if(t == 1)
				{
					/*determine the pixel that the sample contributes to*/
					glm::dvec3 rayW = light_subpath[s -1].sp.pos - eye_subpath[0].sp.pos;
					glm::dvec3 rayV = glm::normalize(glm::dvec3(m_V * glm::dvec4(rayW, 0)));
					double d = m_image_plane_distance / rayV.z;

					glm::dvec3 lens_posV = glm::dvec3(m_V * glm::dvec4(eye_subpath[0].sp.pos, 1));
					glm::dvec3 ipV = lens_posV + d * rayV;

					double pu = 0.5 * ((ipV.x / m_image_plane_ratio) + 1);
					double pv = 1.0 - 0.5 * (ipV.y + 1);

					/*if the intersection point is not within the image plane then move on to the next sample*/
					if((pu < 0) || (pu > 1) || (pv < 0) || (pv > 1))
						continue;

					/*otherwise compute the intersected pixel coordinates*/
					t1_pixel_x = std::floor(static_cast<double>(m_img_res_x - 1) * pu);
					t1_pixel_y = std::floor(static_cast<double>(m_img_res_y - 1) * pv);
				}

				glm::dvec3 c;
				if(!connectionFactor(light_subpath, eye_subpath, s, t, c))
					continue;

				double w = weight(light_subpath, eye_subpath, s, t);

				if(t == 1)
				{
					glm::dvec3 I_t1 = light_subpath[s-1].a * eye_subpath[0].a * c * w;

					std::lock_guard<std::mutex> lock(m_pixel_buffer_mutex);
					m_pixel_buffer->at(t1_pixel_x, t1_pixel_y) += I_t1;
				}
				else
					I += light_subpath[s-1].a * eye_subpath[t-1].a * c * w;
			}
		}

	/*update accumulated measurement value in the pixel buffer*/
	std::lock_guard<std::mutex> lock(m_pixel_buffer_mutex);
	m_pixel_buffer->at(px, py) += I;
}

glm::dvec3 UBDPTRenderer::s0sample(const std::vector<UPathVertex>& subpath, const UPathVertex& emitter_vertex)
{
	if(emitter_vertex.sp.object == nullptr)
		return {0, 0, 0};
	if(!emitter_vertex.sp.object->isEmitter())
		return {0, 0, 0};

	const std::shared_ptr<UEmitter>& e = std::dynamic_pointer_cast<UEmitter>(emitter_vertex.sp.object);

	const UPathVertex& curr_vertex = subpath.back();

	/*compute weight*/
	double w = 1.0;
	/*probability of generating the last vertex (next vertex)*/
	double ratio = 1.0;

	ratio = (e->probability() / e->area()) / emitter_vertex.p_eye_A;

	if(!curr_vertex.specular)
		w += std::pow(ratio, 2);

	/*probability of generating the penultimate vertex (curr vertex)*/

	glm::dvec3 edge = curr_vertex.sp.pos - emitter_vertex.sp.pos;
	double edge_length2 = glm::dot(edge, edge);
	edge /= std::sqrt(edge_length2);

	double d1 = glm::dot(emitter_vertex.sp.Ns, edge);
	double d2 = glm::dot(curr_vertex.sp.Ns, -edge);
	double G = std::abs(d1 * d2) / edge_length2;

	double p_light_psa = 1.0 / (2.0 * M_PI * std::abs(d1));

	ratio *= (p_light_psa * G) / curr_vertex.p_eye_A;

	if(!curr_vertex.specular)
	{
		if(subpath.size() > 1)
		{
			if(!subpath[subpath.size() - 2].specular)
				w += std::pow(ratio, 2);
		}
		else
			w += std::pow(ratio, 2);
	}

	if(subpath.size() > 2)
		for(int v = subpath.size()-2; v > 0; v--)
		{
			ratio *= subpath[v].p_light_A / subpath[v].p_eye_A;

			if(subpath[v].specular)
				continue;
			if(v > 0)
				if(subpath[v - 1].specular)
					continue;

			w += std::pow(ratio, 2);
		}

	w = 1.0 / w;

	glm::dvec3 c = (e->power() / e->area()) * p_light_psa;
	glm::dvec3 I = w * c * emitter_vertex.a;

	return I;
}

glm::dvec3 UBDPTRenderer::computeEyeSubpath(std::vector<UPathVertex>& subpath, size_t px, size_t py, size_t lens_sample_id, size_t pixel_sample_id)
{
	subpath.clear();

	/*accumulated measurement for s=0 samples*/
	glm::dvec3 I = glm::dvec3(0);

	/*compute point on the lens's surface*/
	glm::dvec3 lens_pointV = glm::dvec3(m_lens_radius*URng::get().sampleUnitDiskStratified(m_num_lens_strata, lens_sample_id), 0);

	/*generate and add the vertex at the lens's surface to the subpath*/
	UPathVertex lens_vertex{};
	lens_vertex.a = glm::dvec3(m_W);
	lens_vertex.sp.pos = glm::dvec3(m_invV * glm::dvec4(lens_pointV, 1.0));
	lens_vertex.sp.Ns = lens_vertex.sp.Ng = transformVector(m_invV, glm::dvec3(0, 0, 1));
	lens_vertex.sp.Ts = transformVector(m_invV, glm::dvec3(1, 0, 0));
	lens_vertex.sp.Bs = transformVector(m_invV, glm::dvec3(0, 1, 0));
	lens_vertex.p_eye_A = 1.0 / m_lens_area;
	lens_vertex.specular = false;

	subpath.push_back(lens_vertex);

	/*compute a point on the pixel surface to cast a ray through*/
	glm::dvec2 pixel_point = URng::get().sampleUnitRectStratified(m_num_pixel_strata, pixel_sample_id);
	glm::dvec3 image_pointV = glm::dvec3( -m_image_plane_ratio + (px + pixel_point.x) * m_pixel_width,
											1.0 - (py + pixel_point.y) * m_pixel_height,
											m_image_plane_distance);
	image_pointV = glm::normalize(image_pointV);

	glm::dvec3 foucs_plane_pointV = image_pointV * (m_focus_plane_distance / image_pointV.z);
	glm::dvec3 eye_ray_dirW = transformVector(m_invV, foucs_plane_pointV - lens_pointV);

	/*generate the initial ray to cast from the lens's surface*/
	URay ray = URay(lens_vertex.sp.pos, eye_ray_dirW);

	/*cast the first eye ray and find the closest intersection;
	* if none is found, terminate*/
	UPathVertex next_vertex{};
	if(!m_scene->intersectionPoint(ray, next_vertex.sp))
		return glm::dvec3(0);

	next_vertex.a = lens_vertex.a;
	/*points on the lens's surface are chosen uniformly with respect to its area*/
	next_vertex.p_eye_A = 1.0 / m_image_plane_area;

	while(true)
	{
		/*if light the bsdf is set to nullptr then the path is terminated
		* and the vertex is not added to the subpath as no light is scattered at it anyway*/
		if(next_vertex.sp.bsdf == nullptr)
		{
			/*if the next vertex is on an emitter's surface, we still need to evaluate the s=0 sample*/
			/*convert emitter vertex position and normal to world coordinates to properly compute the sample*/
			next_vertex.sp.pos = transformPoint(next_vertex.sp.W, next_vertex.sp.pos);
			next_vertex.sp.Ns = transformVectorT(next_vertex.sp.invW, next_vertex.sp.Ns);
			/*if the new vertex is an emitter, compute its emitted radiance (s=0 sample), before terminating*/
			I += s0sample(subpath, next_vertex);

			break;
		}

		/*---compute new ray's direction---*/
		/*construct a TNB matrix which maps from tangent space to object's local space*/
		glm::mat3x3 TNB;
		TNB[0] = next_vertex.sp.Ts;
		TNB[1] = next_vertex.sp.Ns;
		TNB[2] = next_vertex.sp.Bs;

		UBsdfSurfaceInfo scatter_info = UBsdfSurfaceInfo::fromSurfacePoint(next_vertex.sp);
		/*move the direction from world space to local space as all other vectors are in local space*/
		glm::dvec3 w = transformVector(next_vertex.sp.invW, -ray.dir());

		double p_psa;
		glm::dvec3 fs;
		glm::dvec3 next_dirT;
		if(!next_vertex.sp.bsdf->scatter(scatter_info, w, next_dirT, p_psa, fs, next_vertex.specular))
			break;

		/*if the new ray goes into the object, flip normals to also point into the object*/
		if(next_dirT.y < 0)
		{
			next_vertex.sp.Ng *= -1.0;
			next_vertex.sp.Ns *= -1.0;
			next_vertex.sp.Ts *= -1.0;
			next_vertex.sp.Bs *= -1.0;
		}

		/*offset position to avoid self intersection; use the geometric normal
		 *  instead of the shading normal to avoid artifacts*/
		next_vertex.sp.pos += 0.00001 * next_vertex.sp.Ng;

		/*convert the position and tangent space vectors to world space*/
		next_vertex.sp.pos = transformPoint(next_vertex.sp.W, next_vertex.sp.pos);
		next_vertex.sp.Ng = transformVectorT(next_vertex.sp.invW, next_vertex.sp.Ng);
		next_vertex.sp.Ns = transformVectorT(next_vertex.sp.invW, next_vertex.sp.Ns);
		next_vertex.sp.Ts = transformVectorT(next_vertex.sp.invW, next_vertex.sp.Ts);
		next_vertex.sp.Bs = transformVectorT(next_vertex.sp.invW, next_vertex.sp.Bs);

		/*if the current vertex is an emitter, compute its emitted radiance (s=0 sample);
		* the emitter position is already in world coordinates at this point*/
		I += s0sample(subpath, next_vertex);

		/*add the new vertex to the subpath*/
		subpath.push_back(next_vertex);
		/*the new vertex becomes the current vertex*/
		UPathVertex& curr_vertex = subpath.back();

		/*set the new ray's origin and position in world space*/
		ray.setOrigin(curr_vertex.sp.pos);
		ray.setDir(transformVector(curr_vertex.sp.W, TNB * next_dirT));

		/*cast the new ray to find the closest intersectio; terminate if none found*/
		if(!m_scene->intersectionPoint(ray, next_vertex.sp))
			break;

		UPathVertex& prev_vertex = subpath[subpath.size() - 2];

		/*---randomly decide whether to generate the next vertex---*/

		double fs_sum = fs.x + fs.y + fs.z;

		/*check if the bsdf and probability density are greater than 0*/
		if(!(p_psa > 0) || !(fs_sum > 0))
			break;

		/*probability for generating the next vertex*/
		double q;

		if(subpath.size() < m_min_depth)
			q = 1.0;
		else
		{
			q = std::min(1.0, (fs_sum / 3.0) / p_psa);
			if(URng::get().unitRand() > q)
				break;
		}

		/*check if the current vertex is specular, as that case requires special treatment*/
		if(curr_vertex.specular)
		{
			next_vertex.a = curr_vertex.a * fs;

			next_vertex.p_eye_A = q * p_psa;
			prev_vertex.p_light_A = q * p_psa;
		}
		else
		{
			glm::dvec3 a = fs / p_psa;

			/*geometric factor to convert the probability densities from projected solid angle measure
			* to area measure; referring to the previous and the next vertex in the path respectively*/
			double G_prev, G_next;
			double d1,d2, edge_length2;
			glm::dvec3 edge;

			/*compute G_prev*/
			edge = prev_vertex.sp.pos - curr_vertex.sp.pos;
			edge_length2 = glm::dot(edge, edge);
			edge /= std::sqrt(edge_length2);
			d1 = glm::dot(edge, curr_vertex.sp.Ns);
			d2 = glm::dot(-edge, prev_vertex.sp.Ns);

			G_prev = std::abs(d1 * d2) / edge_length2;

			/*compute G_next*/
			edge = next_vertex.sp.pos - curr_vertex.sp.pos;
			edge_length2 = glm::dot(edge, edge);
			edge /= std::sqrt(edge_length2);
			d1 = glm::dot(edge, curr_vertex.sp.Ns);
			d2 = glm::dot(-edge, next_vertex.sp.Ns);

			G_next = std::abs(d1 * d2) / edge_length2;

			/*set new vertex's data*/
			next_vertex.a = curr_vertex.a * a;
			next_vertex.p_eye_A = q * p_psa * G_next;

			/*set the new probability density for the previous vertex in the subpath*/
			prev_vertex.p_light_A = q * p_psa * G_prev;
		}
	}

	return I;
}

void UBDPTRenderer::computeLightSubpath(std::vector<UPathVertex>& subpath)
{
	subpath.clear();

	double p = URng::get().unitRand();
	std::shared_ptr<UEmitter> emitter;

	/*choose an emitter randomly using the precomputed probabilities based on emitted power*/
	for(const auto& e : m_scene->emitters())
	{
		if(p < e->probability())
		{
			emitter = e;
			break;
		}
		else
		{
			p -= e->probability();
		}
	}

	UEmitterPoint emitter_pointW;
	emitter->randomPoint(emitter_pointW);

	UPathVertex emitter_vertex;
	emitter_vertex.a = emitter->power();
	emitter_vertex.sp.pos = emitter_pointW.pos;
	emitter_vertex.sp.Ng = emitter_pointW.Ng;
	emitter_vertex.sp.Ns = emitter_pointW.Ns;
	emitter_vertex.sp.Ts = emitter_pointW.Ts;
	emitter_vertex.sp.Bs = emitter_pointW.Bs;
	emitter_vertex.p_light_A = emitter->probability() * (1.0 / emitter->area());
	emitter_vertex.specular = false;

	subpath.push_back(emitter_vertex);

	/*choose random emission direction*/
	glm::dvec3 dirT = URng::get().samplePosHemUniform();

	glm::mat3x3 TNB;
	TNB[0] = emitter_pointW.Ts;
	TNB[1] = emitter_pointW.Ns;
	TNB[2] = emitter_pointW.Bs;

	glm::dvec3 dirW = TNB * dirT;
	URay ray(emitter_pointW.pos, dirW);

	UPathVertex next_vertex{};
	if(!m_scene->intersectionPoint(ray, next_vertex.sp))
		return;

	if(next_vertex.sp.bsdf == nullptr)
		return;

	glm::dvec3 edge = next_vertex.sp.pos - emitter_pointW.pos;
	double edge_length2 = glm::dot(edge, edge);
	edge /= std::sqrt(edge_length2);
	double d1 = glm::dot(edge, emitter_pointW.Ns);
	double d2 = glm::dot(-edge, next_vertex.sp.Ns);

	double G = std::abs(d1 * d2) / edge_length2;

	next_vertex.a = emitter_vertex.a;
	next_vertex.p_light_A = (1 / (2.0 * M_PI * dirT.y)) * G;

	while(true)
	{
		/*---compute new ray's direction---*/
		/*construct a TNB matrix which maps from tangent space to object's local space*/
		TNB[0] = next_vertex.sp.Ts;
		TNB[1] = next_vertex.sp.Ns;
		TNB[2] = next_vertex.sp.Bs;

		UBsdfSurfaceInfo scatter_info = UBsdfSurfaceInfo::fromSurfacePoint(next_vertex.sp);
		/*move the direction from world space to local space as all other vectors are in local space*/
		glm::dvec3 w = transformVector(next_vertex.sp.invW, -ray.dir());

		double p_psa;
		glm::dvec3 fs;
		glm::dvec3 next_dirT;
		if(!next_vertex.sp.bsdf->scatter(scatter_info, w, next_dirT, p_psa, fs, next_vertex.specular))
			break;

		/*if the new ray goes into the object, flip normals to also point into the object*/
		if(next_dirT.y < 0)
		{
			next_vertex.sp.Ng *= -1.0;
			next_vertex.sp.Ns *= -1.0;
			next_vertex.sp.Ts *= -1.0;
			next_vertex.sp.Bs *= -1.0;
		}

		/*offset position to avoid self intersection; use the geometric normal
		 *  instead of the shading normal to avoid artifacts*/
		next_vertex.sp.pos += 0.00001 * next_vertex.sp.Ng;

		/*convert the position and tangent space vectors to world space*/
		next_vertex.sp.pos = transformPoint(next_vertex.sp.W, next_vertex.sp.pos);
		next_vertex.sp.Ng = transformVectorT(next_vertex.sp.invW, next_vertex.sp.Ng);
		next_vertex.sp.Ns = transformVectorT(next_vertex.sp.invW, next_vertex.sp.Ns);
		next_vertex.sp.Ts = transformVectorT(next_vertex.sp.invW, next_vertex.sp.Ts);
		next_vertex.sp.Bs = transformVectorT(next_vertex.sp.invW, next_vertex.sp.Bs);

		/*add the new vertex to the subpath*/
		subpath.push_back(next_vertex);
		/*the new vertex becomes the current vertex*/
		UPathVertex& curr_vertex = subpath.back();

		/*set the new ray's origin and position in world space*/
		ray.setOrigin(curr_vertex.sp.pos);
		ray.setDir(transformVector(curr_vertex.sp.W, TNB * next_dirT));

		/*cast the new ray to find the closest intersectio; terminate if none found*/
		if(!m_scene->intersectionPoint(ray, next_vertex.sp))
			break;

		/*if light the bsdf is set to nullptr then the path is terminated
		* and the vertex is not added to the subpath as no light is scattered at it anyway*/
		if(next_vertex.sp.bsdf == nullptr)
			break;

		UPathVertex& prev_vertex = subpath[subpath.size() - 2];

		/*---randomly decide whether to generate the next vertex---*/

		double fs_sum = fs.x + fs.y + fs.z;

		/*check if the bsdf and probability density are greater than 0*/
		if(!(p_psa > 0) || !(fs_sum > 0))
			break;

		/*probability for generating the next vertex*/
		double q;

		if(subpath.size() < m_min_depth)
			q = 1.0;
		else
		{
			q = std::min(1.0, (fs_sum / 3.0) / p_psa);
			if(URng::get().unitRand() > q)
				break;
		}

		/*check if the current vertex is specular, as that case requires special treatment*/
		if(curr_vertex.specular)
		{
			next_vertex.a = curr_vertex.a * fs;

			next_vertex.p_light_A = q * 1.0;
			prev_vertex.p_eye_A = q * 1.0;
		}
		else
		{
			glm::dvec3 a = fs / p_psa;

			/*geometric factor to convert the probability densities from projected solid angle measure
			* to area measure; referring to the previous and the next vertex in the path respectively*/
			double G_prev, G_next;

			/*compute G_prev*/
			edge = curr_vertex.sp.pos - prev_vertex.sp.pos;
			edge_length2 = glm::dot(edge, edge);
			edge /= std::sqrt(edge_length2);
			d1 = glm::dot(edge, prev_vertex.sp.Ns);
			d2 = glm::dot(-edge, curr_vertex.sp.Ns);

			G_prev = std::abs(d1 * d2) / edge_length2;

			/*compute G_next*/
			edge = next_vertex.sp.pos - curr_vertex.sp.pos;
			edge_length2 = glm::dot(edge, edge);
			edge /= std::sqrt(edge_length2);
			d1 = glm::dot(edge, curr_vertex.sp.Ns);
			d2 = glm::dot(-edge, next_vertex.sp.Ns);

			G_next = std::abs(d1 * d2) / edge_length2;

			/*set new vertex's data*/
			next_vertex.a = curr_vertex.a * a;
			next_vertex.p_light_A = q * p_psa * G_next;

			/*set the new probability density for the previous vertex in the subpath*/
			prev_vertex.p_eye_A = q * p_psa * G_prev;
		}
	}
}

bool UBDPTRenderer::connectionFactor(const std::vector<UPathVertex>& light_subpath, const std::vector<UPathVertex>& eye_subpath, size_t s, size_t t, glm::dvec3& c)
{
	/*conecting vertices*/
	const UPathVertex& vl = light_subpath[s - 1];
	const UPathVertex& ve = eye_subpath[t - 1];

	/*if either connecting vertex is specular, there's zero probability,
	* that the light is scattered along this path*/
	if(vl.specular || ve.specular)
		return false;

	/*if the connecting vertices aren't mutually visible, the light
	* can't be scattered along this path*/
	if(!m_scene->visibility(vl.sp.pos, ve.sp.pos))
		return false;

	/*connecting edge*/
	glm::dvec3 ce = vl.sp.pos - ve.sp.pos;
	/*connecting edge squared length*/
	double ce_length2 = glm::dot(ce, ce);
	/*normalize the connecting edge to obtain unit vector pointing from one vertex to the other*/
	ce /= std::sqrt(ce_length2);

	double d1, d2;
	/*first compute the dot products using geometric normals to make the test below;
	* using shading normals here could produce artifacts*/
	d1 = glm::dot(ce, ve.sp.Ng);
	d2 = glm::dot(-ce, vl.sp.Ng);

	if((d1 <= 0) || (d2 <= 0))
		return false;

	/*we use the shading normals for the actual computation so we recalculate the dot products*/
	d1 = glm::dot(ce, ve.sp.Ns);
	d2 = glm::dot(-ce, vl.sp.Ns);

	/*transformation operator from projected solid angle measure to area measure*/
	double G = (d1 * d2) / ce_length2;

	/*densities of radiance scattered between vertices at the connecting edge
	* equal to bsdf, except when evaluated at the emitter's surface / lens' surface
	* (projected solid angle measure)*/
	glm::dvec3 fs1, fs2;

	glm::dmat3x3 TNB;

	/*fs1*/
	if(s == 1)
		//for now we assume all emitters are lambertian so 1/2pi
		//we dvide by the cosine factor to transform from solid angle measure to projected solid angle measure
		fs1 = glm::dvec3(1) * (1.0 / (2.0 * M_PI * d2));
	else
	{
		TNB[0] = vl.sp.Ts;
		TNB[1] = vl.sp.Ns;
		TNB[2] = vl.sp.Bs;

		fs1 = vl.sp.bsdf->samplePSA(UBsdfSurfaceInfo::fromSurfacePoint(vl.sp),
				  glm::normalize((light_subpath[s - 2].sp.pos - light_subpath[s - 1].sp.pos) * TNB),
				glm::normalize((eye_subpath[t - 1].sp.pos - light_subpath[s - 1].sp.pos) * TNB));

		if(fs1.x + fs1.y + fs1.z  <= 0)
			return false;
	}

	/*fs2*/
	if(t == 1)
	{
		/*find the distance to the edge's intersection point with the image plane*/
		glm::dvec3 edgeV = glm::dvec3(m_V * glm::dvec4(ce, 0));
		double d = m_image_plane_distance / edgeV.z;

		//d1_image_plane = d1
		double d2_image_plane = glm::dot(-edgeV, glm::dvec3(0, 0, -1));

		double G_image_plane = std::abs(d1 * d2_image_plane) / (d * d);
		fs2 = glm::dvec3(1) * ((1.0 / (m_image_plane_area)) / G_image_plane);
	}
	else
	{
		TNB[0] = ve.sp.Ts;
		TNB[1] = ve.sp.Ns;
		TNB[2] = ve.sp.Bs;

		fs2 = ve.sp.bsdf->samplePSA(UBsdfSurfaceInfo::fromSurfacePoint(ve.sp),
				  glm::normalize((light_subpath[s - 1].sp.pos - eye_subpath[t - 1].sp.pos) * TNB),
				glm::normalize((eye_subpath[t-2].sp.pos - eye_subpath[t - 1].sp.pos) * TNB));

		if(fs2.x + fs2.y + fs2.z <= 0)
			return false;
	}

	c = fs1 * fs2 * G;

	return true;
}

double UBDPTRenderer::p_sp1(const std::vector<UPathVertex>& light_subpath, const std::vector<UPathVertex>& eye_subpath, size_t s, size_t t)
{
	double ratio;

	/*compute the probability of generating direction from the last light subpath vertex to the last eye subpath vertex*/
	/*conecting vertices*/
	const UPathVertex& vl = light_subpath[s - 1];
	const UPathVertex& ve = eye_subpath[t - 1];

	/*connecting edge*/
	glm::dvec3 ce = vl.sp.pos - ve.sp.pos;
	/*connecting edge squared length*/
	double ce_length2 = glm::dot(ce, ce);
	/*normalize the connecting edge to obtain unit vector from one vertex to the other*/
	ce /= std::sqrt(ce_length2);

	double d1 = glm::dot(ce, ve.sp.Ns);
	double d2 = glm::dot(-ce, vl.sp.Ns);

	double G = (d1 * d2) / ce_length2;

	if(s == 1)
	{
		ratio = (1.0 / (2.0 * M_PI * d2));
	}
	else //if s > 1
	{
		glm::dmat3x3 TNB;
		TNB[0] = vl.sp.Ts;
		TNB[1] = vl.sp.Ns;
		TNB[2] = vl.sp.Bs;

		glm::dvec3 wsT = glm::normalize(eye_subpath[t - 1].sp.pos - light_subpath[s - 1].sp.pos) * TNB;
		glm::dvec3 wgT = glm::normalize(light_subpath[s - 2].sp.pos - light_subpath[s - 1].sp.pos) * TNB;

		ratio = vl.sp.bsdf->pPSA(UBsdfSurfaceInfo::fromSurfacePoint(vl.sp), wsT, wgT);
	}

	/*transform from projected solid angle measure to area measure*/
	ratio *= G;

	return ratio / eye_subpath[t - 1].p_eye_A;
}

double UBDPTRenderer::p_sm1(const std::vector<UPathVertex>& light_subpath, const std::vector<UPathVertex>& eye_subpath, size_t s, size_t t)
{
	double p_eye_A;

	/*compute the probability of generating direction from the last eye subpath vertex to the last light subpath vertex*/
	if(t == 1)
	{
		p_eye_A = (1.0 / m_image_plane_area);
	}
	else
	{
		/*conecting vertices*/
		const UPathVertex& vl = light_subpath[s - 1];
		const UPathVertex& ve = eye_subpath[t - 1];
		glm::dmat3x3 TNB;

		/*connecting edge*/
		glm::dvec3 ce = vl.sp.pos - ve.sp.pos;
		/*connecting edge squared length*/
		double ce_length2 = glm::dot(ce, ce);
		/*normalize the connecting edge to obtain unit vector from one vertex to the other*/
		ce /= std::sqrt(ce_length2);

		double d1 = glm::dot(ce, ve.sp.Ns);
		double d2 = glm::dot(-ce, vl.sp.Ns);

		double G = (d1 * d2) / ce_length2;

		TNB[0] = ve.sp.Ts;
		TNB[1] = ve.sp.Ns;
		TNB[2] = ve.sp.Bs;

		glm::dvec3 wsT = glm::normalize(light_subpath[s - 1].sp.pos - eye_subpath[t - 1].sp.pos) * TNB;
		glm::dvec3 wgT = glm::normalize(eye_subpath[t - 2].sp.pos - eye_subpath[t - 1].sp.pos) * TNB;

		p_eye_A = (ve.sp.bsdf->pPSA(UBsdfSurfaceInfo::fromSurfacePoint(ve.sp), wsT, wgT) * G);
	}


	return (p_eye_A / light_subpath[s - 1].p_light_A);
}

double UBDPTRenderer::weight(const std::vector<UPathVertex>& light_subpath, const std::vector<UPathVertex>& eye_subpath, size_t s, size_t t)
{
	/*---using power heuristic with B=2 for sample combination---*/

	/* p_s/p_s = 1 , so we start with weight = 1*/
	double w = 1.0;
	double ratio = 1.0;

	/*compute p_s+1; t > 1, because we don't consider paths with t=0;
	* we check whether the previous vertex in the subpath is specular,
	* because then the ratio is zero*/
	if(t > 1)
	{
		ratio = p_sp1(light_subpath, eye_subpath, s, t);

		if(!eye_subpath[t - 2].specular)
			w += std::pow(ratio, 2);
	}

	/*compute p_i for i > s+1*/

	/*we ignore the paths with t=0, so we don't consider them here
	* hence "v > 0"; we would use "v >= 0" otherwise*/
	if(t > 2)
		for(int v = t-2; v > 0; v--)
		{
			ratio *= eye_subpath[v].p_light_A / eye_subpath[v].p_eye_A;

			if(eye_subpath[v].specular)
				continue;
			if(v > 0)
				if(eye_subpath[v - 1].specular)
					continue;

			w += std::pow(ratio, 2);
		}

	/*compute p_s-1; we check the previous vertex in the light subpath is specular,
	 * because then the ratio is zero*/
	ratio = p_sm1(light_subpath, eye_subpath, s, t);

	if(s > 1)
	{
		if(!light_subpath[s - 2].specular)
			w += std::pow(ratio, 2);
	}
	else
		w += std::pow(ratio, 2);

	/*compute p_i for i < s-1*/
	if(s > 1)
		for(int v = s-2; v >= 0; v--)
		{
			ratio *= light_subpath[v].p_eye_A / light_subpath[v].p_light_A;

			if(light_subpath[v].specular)
				continue;
			if(v > 0)
				if(light_subpath[v - 1].specular)
					continue;

			w += std::pow(ratio, 2);
		}

	return (1.0 / w);
}
