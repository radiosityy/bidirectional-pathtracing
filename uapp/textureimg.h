#ifndef TEXTUREIMG_H
#define TEXTUREIMG_H

#include <utexture.h>

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

class TextureImg : public UTexture
{
public:
    static std::shared_ptr<TextureImg> get(const std::string& pathname, bool& ok);

    glm::dvec3 sample(double u, double v) override;

private:
    bool loadImage(const std::string& pathname);

    size_t m_width;
    size_t m_height;
    std::vector<glm::dvec3> m_pixels;

    static std::unordered_map<std::string, std::shared_ptr<TextureImg>> m_textures;
};

#endif // TEXTUREIMG_H
