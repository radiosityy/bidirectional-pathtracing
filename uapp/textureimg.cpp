#include "textureimg.h"

#include <QImage>

std::unordered_map<std::string, std::shared_ptr<TextureImg>> TextureImg::m_textures;

bool TextureImg::loadImage(const std::string& pathname)
{
    /*Load image from file*/
    QImage img(pathname.c_str());
    if(img.isNull())
        return false;

    m_width = img.width();
    m_height = img.height();

    /*convert to suitable format*/
    //img = img.scaled(size_x, size_y, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    img = img.convertToFormat(QImage::Format_RGBA8888);

    /*Convert integer values <0;255> to doubles <0;1> and fill memory*/
    uint8_t* src_pix = img.bits();

    m_pixels.resize(m_width * m_height);

    for(size_t p = 0; p < m_width * m_height; p++)
    {
        m_pixels[p] = glm::dvec3(
                    static_cast<double>(src_pix[p*4]) / 255.0,
                    static_cast<double>(src_pix[p*4 + 1]) / 255.0,
                    static_cast<double>(src_pix[p*4 + 2]) / 255.0
                          );
    }

    return true;
}

std::shared_ptr<TextureImg> TextureImg::get(const std::string& pathname, bool& ok)
{
    auto itr = m_textures.find(pathname);

    if(itr != m_textures.end())
        return std::shared_ptr<TextureImg>(itr->second);
    else
    {
//        TextureImg* t = new TextureImg(name);
//        auto ptr = std::shared_ptr<TextureImg>(t);
        std::shared_ptr<TextureImg> tex = std::make_shared<TextureImg>();
        ok = tex->loadImage(pathname);
        m_textures.insert({pathname, tex});

        return tex;
    }
}

glm::dvec3 TextureImg::sample(double u, double v)
{
    /*clamp tex coords to <0;1>*/
    u = u - size_t(u);
    v = v - size_t(u);

    double x = u * static_cast<double>(m_width - 1);
    double y = v * static_cast<double>(m_height - 1);

    size_t x1 = static_cast<size_t>(std::floor(x));
    size_t x2 = static_cast<size_t>(std::ceil(x));
    size_t y1 = static_cast<size_t>(std::floor(y));
    size_t y2 = static_cast<size_t>(std::ceil(y));

    double tx = x - static_cast<double>(x1);
    double ty = y - static_cast<double>(y1);

    assert((m_width*y1 + x1) < m_pixels.size());
    assert((m_width*y1 + x2) < m_pixels.size());
    assert((m_width*y2 + x1) < m_pixels.size());
    assert((m_width*y2 + x2) < m_pixels.size());

    glm::dvec3 c1 = tx * m_pixels[m_width*y1 + x1] + (1.0 - tx) * m_pixels[m_width*y1 + x2];
    glm::dvec3 c2 = tx * m_pixels[m_width*y2 + x1] + (1.0 - tx) * m_pixels[m_width*y2 + x2];

    return ty * c1 + (1.0f - ty) * c2;
}

