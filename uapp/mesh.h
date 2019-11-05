#ifndef MESH_H
#define MESH_H

#include <string>
#include <vector>
#include <array>

#include <umath.h>
#include "model.h"

#include <assimp/scene.h>

struct MeshVertex
{
    glm::dvec3 pos;
    double tex_u;
    glm::dvec3 normal;
    double tex_v;
    glm::dvec3 tangent;
};

using MeshFace = std::array<MeshVertex, 3>;

class Mesh : public Model
{
public:
    Mesh(aiMesh*);

    void computeFacesProbabilities();

    bool localIntersection(const URay& rayL, USurfacePoint& sp, double& d) override;
    bool intersects(const URay& rayL, double& d) override;
    double area(const glm::dmat4x4& W) override;
    void localRandomPoint(UEmitterPoint& sp) override;

private:
    void computeBoundingSphere();

    std::vector<MeshFace> m_faces;
    std::vector<double> m_faces_probabilities;

    glm::dmat4x4 m_bounding_sphere;
};

#endif // MESH_H
