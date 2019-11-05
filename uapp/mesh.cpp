#include "mesh.h"

Mesh::Mesh(aiMesh* mesh)
{
    m_faces.clear();

    aiVector3D* positions = mesh->mVertices;
    aiVector3D* normals = mesh->mNormals;
    aiVector3D* tangents = mesh->mTangents;
    aiVector3D** textureCoords = mesh->mTextureCoords;

    m_faces.resize(mesh->mNumFaces);

    for(size_t f = 0; f < mesh->mNumFaces; f++)
    {
        for(size_t v = 0; v < 3; v++)
        {
            size_t id = mesh->mFaces[f].mIndices[v];

            auto pos = positions[id];
            m_faces[f][v].pos = glm::dvec3(pos.x, pos.y, pos.z);

            auto normal = normals[id];
            m_faces[f][v].normal = glm::dvec3(normal.x, normal.y, normal.z);

            auto tangent = tangents[id];
            m_faces[f][v].tangent = glm::dvec3(tangent.x, tangent.y, tangent.z);

            m_faces[f][v].tex_u = textureCoords[0][id].x;
            m_faces[f][v].tex_v = textureCoords[0][id].y;
        }
    }

    computeFacesProbabilities();
    computeBoundingSphere();
}

void Mesh::computeBoundingSphere()
{
    double max_d = 0;
    glm::dvec3 x = m_faces[0][0].pos;
    glm::dvec3 y, z;

    for(const auto& f : m_faces)
    {
        for(size_t v = 0; v < 3; v++)
        {
            double d = glm::distance(f[v].pos, x);
            if(d > max_d)
            {
                max_d = d;
                y = f[v].pos;
            }
        }
    }

    max_d = 0;

    for(const auto& f : m_faces)
    {
        for(size_t v = 0; v < 3; v++)
        {
            double d = glm::distance(f[v].pos, y);
            if(d > max_d)
            {
                max_d = d;
                z = f[v].pos;
            }
        }
    }

    glm::dvec3 center = 0.5 * (y + z);
    double radius = 0.5 * max_d;

    for(const auto& f : m_faces)
    {
        for(size_t v = 0; v < 3; v++)
        {
            double d = glm::distance(f[v].pos, center);
            if(d > radius)
                radius = d;
        }
    }

    m_bounding_sphere = glm::inverse(glm::scale(glm::translate(glm::dmat4x4(), center), glm::dvec3(radius)));
}

void Mesh::computeFacesProbabilities()
{
    double total_area = 0;
    m_faces_probabilities.resize(m_faces.size());
    std::vector<double> areas(m_faces.size());

    for(size_t f = 0; f < m_faces.size(); f++)
    {
        double a = triangleArea(m_faces[f][0].pos, m_faces[f][1].pos, m_faces[f][2].pos);

        areas[f] = a;
        total_area += a;
    }

    for(size_t f = 0; f < m_faces.size(); f++)
    {
        m_faces_probabilities[f] = areas[f] / total_area;
    }
}

bool Mesh::localIntersection(const URay& rayL, USurfacePoint& sp, double& sp_d)
{
    struct TriangleIntersection
    {
        double d = std::numeric_limits<double>::infinity();
        double u,v;
        size_t face_id;
    } ci; //closest intersection

    double d, u, v;
    bool hit = false;

    if(!rayL.transform(m_bounding_sphere).intersectUnitSphere(d))
        return false;

    for(size_t f = 0; f < m_faces.size(); f++)
    {
        MeshFace& face = m_faces[f];
        if(rayL.intersectTriangle(face[0].pos, face[1].pos, face[2].pos, d, u, v))
        {
            if(d < ci.d)
            {
                ci.d = d;
                ci.u = u;
                ci.v = v;
                ci.face_id = f;
                hit = true;
            }
        }
    }

    if(!hit)
    {
        return false;
    }

    sp_d = ci.d;

    MeshFace& f = m_faces[ci.face_id];

    sp.tex_u = (1.0 - ci.u - ci.v)*f[0].tex_u + ci.u*f[1].tex_u + ci.v*f[2].tex_u;
    sp.tex_v = (1.0 - ci.u - ci.v)*f[0].tex_v + ci.u*f[1].tex_v + ci.v*f[2].tex_v;
    sp.pos = rayL.origin() + ci.d * rayL.dir();
    sp.Ns = glm::normalize((1.0 - ci.u - ci.v)*f[0].normal + ci.u*f[1].normal + ci.v*f[2].normal);
    sp.Ng = glm::normalize(glm::cross(f[1].pos - f[0].pos, f[2].pos - f[0].pos));
    if(glm::dot(sp.Ns, sp.Ng) < 0)
        sp.Ng *= -1.0;
    sp.Ts = glm::normalize((1 - ci.u - ci.v)*f[0].tangent + ci.u*f[1].tangent + ci.v*f[2].tangent);
    sp.Bs = glm::normalize(glm::cross(sp.Ns, sp.Ts));

    return true;
}

bool Mesh::intersects(const URay& rayL, double& d)
{
    double u,v;
    double min_d = std::numeric_limits<double>::infinity();
    bool hit = false;

    if(!rayL.transform(m_bounding_sphere).intersectUnitSphere(d))
        return false;

    for(MeshFace& f : m_faces)
    {
        if(rayL.intersectTriangle(f[0].pos, f[1].pos, f[2].pos, d, u, v))
        {
            if(d < min_d)
            {
                min_d = d;
                hit = true;
            }
        }
    }

    if(hit)
    {
        d = min_d;
        return true;
    }

    return false;
}

double Mesh::area(const glm::dmat4x4& W)
{
    double A = 0;

    for(const MeshFace& f : m_faces)
    {
        A += triangleArea( glm::dvec3(W * glm::dvec4(f[0].pos, 1.0)),
                glm::dvec3(W * glm::dvec4(f[1].pos, 1.0)),
                glm::dvec3(W * glm::dvec4(f[2].pos, 1.0))
                 );
    }

    return A;
}

void Mesh::localRandomPoint(UEmitterPoint& ep)
{
    double r = URng::get().unitRand();

    for(size_t f = 0; f < m_faces.size(); f++)
    {
        if(r < m_faces_probabilities[f])
        {
            const MeshFace& face = m_faces[f];
            double u, v;

            ep.pos = URng::get().sampleTriangleUniform(face[0].pos, face[1].pos, face[2].pos, u, v);
            ep.Ns = glm::normalize((1.0 - u - v)*face[0].normal + u*face[1].normal + v*face[2].normal);
            ep.Ng = glm::normalize(glm::cross(face[1].pos - face[0].pos, face[2].pos - face[0].pos));
            if(glm::dot(ep.Ns, ep.Ng) < 0)
                ep.Ng *= -1.0;
            ep.Ts = glm::normalize((1.0 - u - v)*face[0].tangent + u*face[1].tangent + v*face[2].tangent);
            ep.Bs = glm::normalize(glm::cross(ep.Ns, ep.Ts));
//            ep.tex_u = (1.0 - u - v)*face[0].tex_u + u*face[1].tex_u + v*face[2].tex_u;
//            ep.tex_v = (1.0 - u - v)*face[0].tex_v + u*face[1].tex_v + v*face[2].tex_v;

            return;
        }
        else
            r -= m_faces_probabilities[f];
    }
}
