#include "scene.h"

#include "implicitsphere.h"
#include "object.h"
#include "emitter.h"
#include "mesh.h"
#include "textureimg.h"
#include "texturecolor.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <iostream>
#include <string>

#include <QFile>

void errorMessage(const std::string& msg)
{
    std::cout << msg << std::endl;
}

void Scene::cameraFromXml(QDomElement& xmlCamera)
{
    double ratio_w, ratio_h, vfov;
    glm::dvec3 pos = glm::dvec3(0, 0, 0);
    glm::dvec3 lookAt = glm::dvec3(0, 0, 1);

    QDomElement xmlRatio = xmlCamera.elementsByTagName("ratio").at(0).toElement();

    ratio_w = xmlRatio.elementsByTagName("w").at(0).firstChild().nodeValue().toDouble();
    ratio_h = xmlRatio.elementsByTagName("h").at(0).firstChild().nodeValue().toDouble();

    vfov = xmlCamera.elementsByTagName("vfov").at(0).firstChild().nodeValue().toDouble();

    if(xmlCamera.elementsByTagName("position").size() != 0)
    {
        QDomElement xmlPos = xmlCamera.elementsByTagName("position").at(0).toElement();
        pos.x = xmlPos.elementsByTagName("x").at(0).firstChild().nodeValue().toDouble();
        pos.y = xmlPos.elementsByTagName("y").at(0).firstChild().nodeValue().toDouble();
        pos.z = xmlPos.elementsByTagName("z").at(0).firstChild().nodeValue().toDouble();
    }

    if(xmlCamera.elementsByTagName("lookAt").size() != 0)
    {
        QDomElement xmlLookAt = xmlCamera.elementsByTagName("lookAt").at(0).toElement();
        lookAt.x = xmlLookAt.elementsByTagName("x").at(0).firstChild().nodeValue().toDouble();
        lookAt.y = xmlLookAt.elementsByTagName("y").at(0).firstChild().nodeValue().toDouble();
        lookAt.z = xmlLookAt.elementsByTagName("z").at(0).firstChild().nodeValue().toDouble();
    }

    m_camera = std::make_unique<UCamera>(ratio_w / ratio_h, vfov, pos, lookAt);
}

void Scene::objectFromXml(const QDomElement& xmlObject)
{
    /*---read texture data---*/
    std::shared_ptr<UTexture> tex = std::make_shared<TextureColor>(glm::dvec3(0.8, 0.8, 0.8));
    auto xmlTex = xmlObject.elementsByTagName("texture");
    if(xmlTex.size() != 0)
    {
        std::string tex_filename = xmlTex.at(0).firstChild().nodeValue().toStdString();

        bool ok;
        tex = TextureImg::get(tex_filename, ok);

        if(!ok)
        {
            errorMessage(std::string("Failed to load texture from file (") + tex_filename + ")");
        }
    }
    else if(xmlObject.elementsByTagName("color").size() != 0)
    {
        auto xmlCol = xmlObject.elementsByTagName("color").at(0).toElement();
        glm::dvec3 col;
        col.r = xmlCol.elementsByTagName("r").at(0).firstChild().nodeValue().toDouble();
        col.g = xmlCol.elementsByTagName("g").at(0).firstChild().nodeValue().toDouble();
        col.b = xmlCol.elementsByTagName("b").at(0).firstChild().nodeValue().toDouble();

        tex = std::make_shared<TextureColor>(col);
    }

    /*---read material data---*/
    std::shared_ptr<Material> mat = std::make_shared<LatexPaint>(tex);

    QString mat_str = xmlObject.elementsByTagName("material").at(0).firstChild().nodeValue();
    if(mat_str == "Glossy")
    {
        double d = xmlObject.elementsByTagName("material").at(0).toElement().attribute("d").toDouble();
        double s = xmlObject.elementsByTagName("material").at(0).toElement().attribute("s").toDouble();
        mat = std::make_shared<Glossy>(tex, d, s);
    }
    else if(mat_str == "PerfectMirror")
        mat = std::make_shared<PerfectMirror>(tex);
    else if(mat_str == "Dielectric")
    {
        double eta = xmlObject.elementsByTagName("material").at(0).toElement().attribute("eta").toDouble();
        mat = std::make_shared<Dielectric>(tex, eta);
    }

    /*---read emission data---*/
    glm::dvec3 P;
    bool emitter = false;
    auto nodesEmit = xmlObject.elementsByTagName("emit");
    if(nodesEmit.size() != 0)
    {
        emitter = true;
        QDomElement xmlEmit = nodesEmit.at(0).toElement();
        P.r = xmlEmit.elementsByTagName("r").at(0).firstChild().nodeValue().toDouble();
        P.g = xmlEmit.elementsByTagName("g").at(0).firstChild().nodeValue().toDouble();
        P.b = xmlEmit.elementsByTagName("b").at(0).firstChild().nodeValue().toDouble();
    }

    /*---read geometry data---*/
    glm::dmat4x4 T;
    std::shared_ptr<Model> model;
    QString type = xmlObject.attribute("type");

    if(type == "implicit_sphere")
    {
        glm::dvec3 center = glm::dvec3(0, 0, 0);
        double radius = 1.0;

        if(xmlObject.elementsByTagName("radius").size() != 0)
            radius = xmlObject.elementsByTagName("radius").at(0).firstChild().nodeValue().toDouble();

        if(xmlObject.elementsByTagName("center").size() != 0)
        {
            QDomElement xmlCenter = xmlObject.elementsByTagName("center").at(0).toElement();
            center.x = xmlCenter.elementsByTagName("x").at(0).firstChild().nodeValue().toDouble();
            center.y = xmlCenter.elementsByTagName("y").at(0).firstChild().nodeValue().toDouble();
            center.z = xmlCenter.elementsByTagName("z").at(0).firstChild().nodeValue().toDouble();
        }

        T = glm::scale(glm::translate(glm::dmat4x4(), center), glm::dvec3(radius));
        model = std::make_shared<ImplicitSphere>();

        if(emitter)
            addEmitter(new Emitter(model, T, mat, P));
        else
            addObject(new Object(model, T, mat));
    }
    else if (type == "mesh")
    {
        QString filename = xmlObject.elementsByTagName("file").at(0).firstChild().nodeValue();

        glm::dvec3 trans = glm::dvec3(0, 0, 0);
        glm::dvec3 rot = glm::dvec3(1, 1, 1);
        glm::dvec3 scale =glm::dvec3(1, 1, 1);
        double rot_angle = 0;

        if(xmlObject.elementsByTagName("translation").size() != 0)
        {
            QDomElement xmlTranslation = xmlObject.elementsByTagName("translation").at(0).toElement();
            trans.x = xmlTranslation.elementsByTagName("x").at(0).firstChild().nodeValue().toDouble();
            trans.y = xmlTranslation.elementsByTagName("y").at(0).firstChild().nodeValue().toDouble();
            trans.z = xmlTranslation.elementsByTagName("z").at(0).firstChild().nodeValue().toDouble();
        }

        if(xmlObject.elementsByTagName("rotation").size() != 0)
        {
            QDomElement xmlRotation = xmlObject.elementsByTagName("rotation").at(0).toElement();
            rot.x = xmlRotation.elementsByTagName("x").at(0).firstChild().nodeValue().toDouble();
            rot.y = xmlRotation.elementsByTagName("y").at(0).firstChild().nodeValue().toDouble();
            rot.z = xmlRotation.elementsByTagName("z").at(0).firstChild().nodeValue().toDouble();
            rot_angle = xmlRotation.elementsByTagName("angle").at(0).firstChild().nodeValue().toDouble();
        }

        if(xmlObject.elementsByTagName("scale").size() != 0)
        {
            QDomElement xmlScale = xmlObject.elementsByTagName("scale").at(0).toElement();
            scale.x = xmlScale.elementsByTagName("x").at(0).firstChild().nodeValue().toDouble();
            scale.y = xmlScale.elementsByTagName("y").at(0).firstChild().nodeValue().toDouble();
            scale.z = xmlScale.elementsByTagName("z").at(0).firstChild().nodeValue().toDouble();
        }

        T = glm::scale(glm::rotate<double>(glm::translate(glm::dmat4x4(), trans), rot_angle, rot), scale);

        std::vector<std::shared_ptr<Model>> models;
        loadObjectFromFile(filename.toStdString(), models);

        for(size_t m = 0; m < models.size(); m++)
        {
            if(emitter)
                addEmitter(new Emitter(models[m], T, mat, P));
            else
                addObject(new Object(models[m], T, mat));
        }
    }
}

bool Scene::fromXml(const std::string& filename)
{
    QDomDocument xmlDoc;
    QFile xmlFile(QString::fromStdString(filename));
    if(!xmlFile.open(QIODevice::ReadOnly))
    {
        errorMessage("Failed to open scene file.");
        return false;
    }

    if(!xmlDoc.setContent(&xmlFile))
    {
        errorMessage("Failed to load xml document.");
        return false;
    }

    xmlFile.close();

    QDomElement xmlRoot = xmlDoc.firstChildElement();

    /*---read camera data---*/
    QDomElement xmlCamera = xmlRoot.elementsByTagName("camera").at(0).toElement();
    cameraFromXml(xmlCamera);

    QDomNodeList xmlObjects = xmlRoot.elementsByTagName("object");
    for(size_t i = 0; i < xmlObjects.size(); i++)
    {
        objectFromXml(xmlObjects.at(i).toElement());
    }

    return true;
}

void Scene::addObject(UObject* o)
{
    m_objects.emplace_back(o);
}

void Scene::addEmitter(UEmitter* e)
{
    std::shared_ptr<UEmitter> ptr = std::shared_ptr<UEmitter>(e);
    m_objects.push_back(ptr);
    m_emitters.push_back(ptr);
}



UCamera& Scene::camera()
{
    return *m_camera;
}

const std::vector<std::shared_ptr<UObject>>& Scene::objects()
{
    return m_objects;
}

const std::vector<std::shared_ptr<UEmitter> >& Scene::emitters()
{
    return m_emitters;
}

bool Scene::loadObjectFromFile(const std::string& filename, std::vector<std::shared_ptr<Model> >& models)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filename.c_str(),
                                        aiProcess_MakeLeftHanded     | \
                                        aiProcess_CalcTangentSpace              |  \
                                        aiProcess_GenSmoothNormals              |   \
                                        aiProcess_JoinIdenticalVertices         |  \
                                        aiProcess_ImproveCacheLocality          |  \
                                        aiProcess_LimitBoneWeights              |  \
                                        aiProcess_RemoveRedundantMaterials      |  \
                                        aiProcess_Triangulate                   |  \
                                        aiProcess_GenUVCoords                   |   \
                                        aiProcess_SortByPType                   |  \
                                        aiProcess_FindDegenerates               |  \
                                        aiProcess_FindInvalidData               |  \
                                        aiProcess_FindInstances                  |  \
                                        aiProcess_ValidateDataStructure          |  \
                                        aiProcess_OptimizeMeshes
                                        );

    if(scene == nullptr)
    {
        errorMessage("Failed to load scene.");
        return false;
    }

    for(size_t m = 0; m < scene->mNumMeshes; m++)
    {
        aiMesh* mesh = scene->mMeshes[m];

        if(!mesh->HasPositions())
        {
            errorMessage("Mesh has no positions.");
            return false;
        }

        if(!mesh->HasFaces())
        {
            errorMessage("Mesh has no faces.");
            return false;
        }

        if(!mesh->HasNormals())
        {
            errorMessage("Mesh has no normals.");
            return false;
        }

        if(!mesh->HasTextureCoords(0))
        {
            errorMessage("Mesh has no texture coords.");
            return false;
        }

        if(!mesh->HasTangentsAndBitangents())
        {
            errorMessage("Mesh has no tangents/bitangents.");
            return false;
        }

        for(size_t f = 0; f < mesh->mNumFaces; f++)
        {
            if(mesh->mFaces[f].mNumIndices != 3)
            {
                errorMessage("Mesh has non triangular faces.");
                return false;
            }
        }

        models.push_back(std::make_shared<Mesh>(mesh));
    }

    return true;
}

