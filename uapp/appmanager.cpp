#include "appmanager.h"

#include <iostream>
#include <map>

QPixmap AppManager::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QPixmap pixmap;

    int width = requestedSize.width() > 0 ? requestedSize.width() : m_img_width;
    int height = requestedSize.height() > 0 ? requestedSize.height() : m_img_height;

    if((width == 0) || (height == 0))
    {
        width = height = 1000;
    }

    if(size != nullptr)
        *size = QSize(width, height);

    if(m_img_rgb.size() > 0)
    {
        QImage img((unsigned char*)m_img_rgb.data(), m_img_width, m_img_height, QImage::Format_RGBA8888);
        pixmap = QPixmap::fromImage(img).scaled(width, height);
    }
    else
    {
        pixmap = QPixmap(width, height);
        pixmap.fill(QColor("black").rgba());
    }

    return pixmap;
}

AppManager::AppManager(QObject *parent) : QObject(parent), QQuickImageProvider(QQuickImageProvider::Pixmap)
{
    m_log_text = "";

    m_curr_pass = 0;
    m_avg_pass_time = 0;
    m_num_threads = 0;

    m_img_width = 0;
    m_img_height = 0;
    m_pixel_subdiv = 0;
    m_lens_subdiv = 0;
    m_lens_size = 0;
    m_focus_plane = 0;
    m_min_depth = 0;

    m_gamma = 2.4;
    m_rgb_format = URgbFormat::sRGB;

    m_running = false;
    m_progress = 0;
}

bool AppManager::initialize()
{
    return true;
}

void AppManager::updateProgress(double progress)
{
    size_t new_progress = static_cast<size_t>(progress * 100.0);

    if(new_progress != m_progress)
    {
        m_progress = new_progress;
        emit progressChanged();
    }
}

void AppManager::log(const std::string& msg)
{
    m_log_text += QString::fromStdString(msg) + "\n";
    emit logTextChanged();
}

void AppManager::logInfo(const std::string& msg)
{
    log("[INFO]  " + msg);
}

void AppManager::logDebug(const std::string& msg)
{
    log("[DEBUG]  " + msg);
}

void AppManager::logError(const std::string& msg)
{
    log("[ERROR]  " + msg);
}

void AppManager::fetchRgbImageFromEngine()
{
    std::lock_guard<std::mutex> lock(m_img_rgb_mutex);

    if((m_img_width == 0) || (m_img_height == 0) || (m_gamma <= 0))
        return;

    std::vector<glm::dvec3> img_data;
    size_t img_res_x, img_res_y;
    if(UEngine::get().imageRGB(img_data, m_rgb_format, m_gamma, img_res_x, img_res_y) != UResult::USuccess)
    {
        logInfo("Unable to convert image to rgb format. Continuing...");
    }
    else
    {
        if((img_res_x != m_img_width) || (img_res_y != m_img_height))
        {
            logError("The image size returned by the engine does not match the cached image size!");
        }

        m_img_rgb.resize(img_res_x * img_res_y * 4);
        for(size_t i = 0; i < img_res_x * img_res_y; i++)
        {
            m_img_rgb[4*i] = static_cast<uint8_t>(img_data[i].r * 255.0);
            m_img_rgb[4*i + 1] = static_cast<uint8_t>(img_data[i].g * 255.0);
            m_img_rgb[4*i + 2] = static_cast<uint8_t>(img_data[i].b * 255.0);
            m_img_rgb[4*i + 3] = 255.0;
        }

        emit previewImgChanged();
    }
}

void AppManager::updateParameterLabels(const URenderParameters& rp)
{
    if(rp.img_res_x != m_img_width)
    {
        m_img_width = rp.img_res_x;
        emit imgWidthChanged();
    }

    if(rp.img_res_y != m_img_height)
    {
        m_img_height = rp.img_res_y;
        emit imgHeightChanged();
    }

    if(rp.pixel_subdiv != m_pixel_subdiv)
    {
        m_pixel_subdiv = rp.pixel_subdiv;
        emit pixelSubdivChanged();
    }

    if(rp.lens_subdiv != m_lens_subdiv)
    {
        m_lens_subdiv = rp.lens_subdiv;
        emit lensSubdivChanged();
    }

    if(rp.lens_size != m_lens_size)
    {
        m_lens_size = rp.lens_size;
        emit lensSizeChanged();
    }

    if(rp.focus_plane_distance != m_focus_plane)
    {
        m_focus_plane = rp.focus_plane_distance;
        emit focusPlaneChanged();
    }

    if(rp.min_depth != m_min_depth)
    {
        m_min_depth = rp.min_depth;
        emit minDepthChanged();
    }
}

void AppManager::newRendering(QList<QString> params)
{
    if(m_render_future.valid())
    {
        auto status = m_render_future.wait_for(std::chrono::duration<double>(0));
        if(status != std::future_status::ready)
        {
            logError("Can't start new rendering. Stop current rendering before starting a new one.");
            return;
        }
    }

    logInfo("Initializing new rendering...");

    URenderParameters rp;
    URendererType rt;
    bool ok;

    rp.img_res_x = params[0].toUInt(&ok);
    if(!ok || (rp.img_res_x == 0))
    {
        logError("Image width must be a positive integer value!");
        return;
    }

    rp.img_res_y = params[1].toUInt(&ok);
    if(!ok || (rp.img_res_y == 0))
    {
        logError("Image height must be a positive integer value!");
        return;
    }

    rp.pixel_subdiv = params[2].toUInt(&ok);
    if(!ok || (rp.pixel_subdiv == 0))
    {
        logError("Pixel subdivision must be a positive integer value!");
        return;
    }

    rp.lens_subdiv = params[3].toUInt(&ok);
    if(!ok || (rp.lens_subdiv == 0))
    {
        logError("Lens subdivision must be a positive integer value!");
        return;
    }

    rp.lens_size = params[4].toDouble(&ok);
    if(!ok || (rp.lens_size <= 0))
    {
        logError("Lens size must be a positive real value!");
        return;
    }

    rp.focus_plane_distance = params[5].toDouble(&ok);
    if(!ok || (rp.focus_plane_distance <= 0))
    {
        logError("Focus plane distance must be a positive real value!");
        return;
    }

    rp.min_depth = params[6].toUInt(&ok);
    if(!ok || (rp.min_depth == 0))
    {
        logError("Minimum depth must be a positive integer value!");
        return;
    }

    auto it = renderer_type_map.find(params[7]);
    if(it == renderer_type_map.end())
    {
        logError("Invalid renderer type specified!");
        return;
    }
    rt = it->second;

    UResult res = UEngine::get().newRendering(rp, rt);

    if(res == UResult::USuccess)
    {
        updateParameterLabels(rp);

        if(m_curr_pass != 0)
        {
            m_curr_pass = 0;
            emit currPassChanged();

            m_avg_pass_time = 0;
            m_total_time = 0;
            emit avgPassTimeChanged();
        }

        if(m_renderer_type != rt)
        {
            m_renderer_type = rt;
            emit rendererTypeChanged();
        }

        double r = static_cast<double>(m_img_width) / static_cast<double>(m_img_height);
        m_scene->camera().setAspectRatio(r);

        logInfo("Done.");
    }
    else
        switch(res)
        {
        case UResult::UInvalidScene:
            logError("Invalid scene. Set a valid scene before starting a new rendering.");
            break;
        case UResult::UError:
            logError("An error occurred.");
            break;
        default:
            break;
        };
}

void AppManager::loadRendering(QString filename)
{
    if(m_render_future.valid())
    {
        auto status = m_render_future.wait_for(std::chrono::duration<double>(0));
        if(status != std::future_status::ready)
        {
            logError("Can't load rendering. Stop current rendering before loading another one!");
            return;
        }
    }

    logInfo("Loading rendering from file... (" + filename.toStdString() + ")");

    URenderParameters params;
    size_t curr_pass;
    URendererType rt;

    UResult res = UEngine::get().loadRendering(filename.toStdString(), params, rt, curr_pass);

    if(res == UResult::USuccess)
    {
        updateParameterLabels(params);

        if(m_curr_pass != curr_pass)
        {
            m_curr_pass = curr_pass;
            emit currPassChanged();
        }

        if(m_renderer_type != rt)
        {
            m_renderer_type = rt;
            emit rendererTypeChanged();
        }

        fetchRgbImageFromEngine();

        logInfo("Done.");
    }
    else switch(res)
    {
    case UResult::UInvalidScene:
        logError("Invalid scene. Set a valid scene before starting a new rendering.");
        break;
    case UResult::UInvalidFormat:
        logError("Failed to load rendering. Invalid file format.");
        break;
    case UResult::UError:
        logError("An error occurred.");
        break;
    default:
        break;
    };
}

void AppManager::loadScene(QString filename)
{
    if(m_render_future.valid())
    {
        auto status = m_render_future.wait_for(std::chrono::duration<double>(0));
        if(status != std::future_status::ready)
        {
            logError("Can't load a new scene. Stop current rendering before loading a new scene.");
            return;
        }
    }

    logInfo("Loading scene... (" + filename.toStdString() + ")");

    m_scene = std::make_shared<Scene>();
    if(!m_scene->fromXml(filename.toStdString()))
    {
        logError(std::string("Failed to load scene from file") + filename.toStdString());
        return;
    }

    UEngine::get().setScene(m_scene);

    m_scene_filename = filename;
    logInfo("Done.");
}

void AppManager::refreshScene()
{
    if(!m_scene_filename.isNull() && !m_scene_filename.isEmpty())
        loadScene(m_scene_filename);
}

void AppManager::saveRendering(QString filename)
{
    if(m_render_future.valid())
    {
        auto status = m_render_future.wait_for(std::chrono::duration<double>(0));
        if(status != std::future_status::ready)
        {
            logError("Can't save rendering. Stop current rendering before saving!");
            return;
        }
    }

    logInfo("Saving rendering to file... (" + filename.toStdString() + ")");
    UResult res = UEngine::get().saveRendering(filename.toStdString());

    if(res != UResult::USuccess)
        logError("Could not save the rendering. An error occurred.");
}

void AppManager::saveImage(QString filename)
{
    std::lock_guard<std::mutex> lock(m_img_rgb_mutex);

    logInfo("Saving rbg image to file... (" + filename.toStdString() + ")");

    if(m_img_rgb.size() == 0)
    {
        logInfo("No image data to save. Aborting.");
        return;
    }

    QImage img((unsigned char*)m_img_rgb.data(), m_img_width, m_img_height, QImage::Format_RGBA8888);

    if(!img.save(filename))
    {
        logError("Failed to save image to file!");
    }

    logInfo("Done.");
}

void AppManager::renderLoop()
{
    logInfo("Staring render loop...");

    m_running = true;
    emit statusChanged();

    while(m_running)
    {
        auto start_timestamp = std::chrono::system_clock::now();

        UResult res = UEngine::get().renderPass(m_num_threads, [&](double progress){updateProgress(progress);});

        if(res == UResult::UUninitialized)
        {
            logError("No renderer set. Start new rendering before running a rendering pass.");
            break;
        }
        if(res == UResult::UStopped)
        {
            logInfo("Rendering stopped.");
            break;
        }
        else
        {
            m_total_time += static_cast<double>(std::chrono::duration<double>(std::chrono::system_clock::now() - start_timestamp).count());
            m_avg_pass_time = m_total_time / static_cast<double>(m_curr_pass + 1);
            emit avgPassTimeChanged();

            m_curr_pass++;
            emit currPassChanged();

            fetchRgbImageFromEngine();
        }
    }

    m_running = false;
    emit statusChanged();
}

void AppManager::startRendering(QString num_threads)
{
    if(m_render_future.valid())
    {
        auto status = m_render_future.wait_for(std::chrono::duration<double>(0));
        if(status != std::future_status::ready)
        {
            logError("Can't start rendering. Stop current rendering before starting a new one!");
            return;
        }
    }

    bool ok;
    size_t nt = num_threads.toUInt(&ok);

    if(!ok || (nt == 0))
    {
        logError("Can't start rendering. Number of threads must be a positive integer!");
        return;
    }

    if(nt != m_num_threads)
    {
        m_num_threads = nt;
        emit numThreadsChanged();
    }

    m_render_future = std::async(std::launch::async, &AppManager::renderLoop, this);
}

void AppManager::stopRendering()
{
    if(!m_running)
        return;

    logInfo("Stopping rendering...");

    m_running = false;
    UEngine::get().stop();
}

void AppManager::onClosing()
{
    stopRendering();

    if(m_render_future.valid())
        m_render_future.wait();
}


/*----------------------Properties Getters and Setters-----------------------*/

QString AppManager::getPreviewImg() const
{
    return "image://app_image_provider/preview_img" + QString::number(m_curr_pass) + QString::number(m_gamma) + QString::number(static_cast<int>(m_rgb_format));
}

QString AppManager::getLogText() const
{
    return m_log_text;
}

QVariantList AppManager::getRendererTypes() const
{
    QVariantList list;

    for(auto it = renderer_type_map.begin(); it != renderer_type_map.end(); it++)
    {
        list.append(it->first);
    }

    return list;
}

QString AppManager::getCurrPass() const
{
    return QString::number(m_curr_pass);
}

QString AppManager::getAvgPassTime() const
{
    return QString::number(m_avg_pass_time, 'g', 4);
}

QString AppManager::getNumThreads() const
{
    return QString::number(m_num_threads);
}

QString AppManager::getStatus() const
{
    if(m_running)
        return "Running";
    else
        return "Not running";
}

bool AppManager::getRunning() const
{
    return m_running;
}

QString AppManager::getProgress() const
{
    return QString::number(m_progress) + "%";
}

double AppManager::getGamma() const
{
    return m_gamma;
}

void AppManager::setGamma(const double& gamma)
{
    m_gamma = gamma;
    fetchRgbImageFromEngine();
}

QVariantList AppManager::getRgbFormats() const
{
    QVariantList list;

    for(auto it = rgb_format_map.begin(); it != rgb_format_map.end(); it++)
    {
        list.append(it->first);
    }

    return list;
}

void AppManager::setRgbFormat(const QString& format)
{
    auto it = rgb_format_map.find(format);
    if(it == rgb_format_map.end())
    {
        logError("UApp:  Invalid rgb format specified!");
        return;
    }

    m_rgb_format = it->second;
    fetchRgbImageFromEngine();
}

QString AppManager::getImgWidth() const
{
    return QString::number(m_img_width);
}

QString AppManager::getImgHeight() const
{
    return QString::number(m_img_height);
}

QString AppManager::getPixelSubdiv() const
{
    return QString::number(m_pixel_subdiv);
}

QString AppManager::getLensSubdiv() const
{
    return QString::number(m_lens_subdiv);
}

QString AppManager::getLensSize() const
{
    return QString::number(m_lens_size);
}

QString AppManager::getFocusPlane() const
{
    return QString::number(m_focus_plane);
}

QString AppManager::getMinDepth() const
{
    return QString::number(m_min_depth);
}

QString AppManager::getRendererType() const
{
    for(auto it = renderer_type_map.begin(); it != renderer_type_map.end(); it++)
    {
        if(it->second == m_renderer_type)
            return it->first;
    }

    return "";
}
