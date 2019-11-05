#ifndef SCENE_H
#define SCENE_H

#include <uscene.h>
#include "model.h"
#include "material.h"

#include <QDomDocument>

class Scene : public UScene
{
public:
    Scene() = default;
    ~Scene() = default;

    bool fromXml(const std::string& filename);

    UCamera& camera() override;
    const std::vector<std::shared_ptr<UObject>>& objects() override;
    const std::vector<std::shared_ptr<UEmitter>>& emitters() override;

private:
    void cameraFromXml(QDomElement&);
    void objectFromXml(const QDomElement&);

    bool loadObjectFromFile(const std::string&, std::vector<std::shared_ptr<Model>>& models);

    void addObject(UObject*);
    void addEmitter(UEmitter*);

    std::unique_ptr<UCamera> m_camera;
    std::vector<std::shared_ptr<UObject>> m_objects;
    std::vector<std::shared_ptr<UEmitter>> m_emitters;
};

#endif // SCENE_H
