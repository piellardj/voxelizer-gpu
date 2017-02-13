#include "IO.hpp"


#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"


bool IO::readObj(std::string const& filename,
                   std::vector<glm::vec3>& vertices,
                   std::vector<glm::ivec3>& triangles)
{
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(shapes, materials, err, filename.c_str())) {
        std::cerr << "Error: couldn't load " << filename << " : " << err << std::endl;
        return false;
    }

    if (!err.empty())
        std::cerr << "Error: while loading " << filename << " : " << err << std::endl;

    for (tinyobj::shape_t const& shape : shapes) {
        if (shape.mesh.indices.size() % 3 != 0) {
            std::cerr << "Warning: in " << filename << ", mesh " << shape.name << " is not triangulated and won't be loaded." << std::endl;
            continue;
        }

        if (shape.mesh.positions.size() % 3 != 0) {
            std::cerr << "Warning: in " << filename << ", some positions are incorrect. The mesh won't be loaded." << std::endl;
             continue;
        }

        unsigned int nbTriangles = shape.mesh.indices.size() / 3;
        unsigned int nbVertices = shape.mesh.positions.size() / 3;

        triangles.resize(nbTriangles);
        for (std::size_t iT = 0 ; iT < nbTriangles ; ++iT) {
            triangles[iT].x = shape.mesh.indices[3*iT + 0];
            triangles[iT].y = shape.mesh.indices[3*iT + 1];
            triangles[iT].z = shape.mesh.indices[3*iT + 2];
        }

        vertices.resize(nbVertices);
        for (std::size_t iV = 0 ; iV < nbVertices ; ++iV) {
            vertices[iV].x = shape.mesh.positions[3*iV + 0];
            vertices[iV].y = shape.mesh.positions[3*iV + 1];
            vertices[iV].z = shape.mesh.positions[3*iV + 2];
        }
    }

    return true;
}

bool IO::readObj(std::string const& filename,
                   std::vector<glm::vec3>& vertices,
                   std::vector<glm::vec3>& normals,
                   std::vector<glm::ivec3>& triangles)
{
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(shapes, materials, err, filename.c_str())) {
        std::cerr << "Error: couldn't load " << filename << " : " << err << std::endl;
        return false;
    }

    if (!err.empty())
        std::cerr << "Error: while loading " << filename << " : " << err << std::endl;

    for (tinyobj::shape_t const& shape : shapes) {
        if (shape.mesh.indices.size() % 3 != 0) {
            std::cerr << "Warning: in " << filename << ", mesh " << shape.name << " is not triangulated and won't be loaded." << std::endl;
            continue;
        }

        if (shape.mesh.positions.size() % 3 != 0) {
            std::cerr << "Warning: in " << filename << ", some positions are incorrect. The mesh won't be loaded." << std::endl;
             continue;
        }

        if (shape.mesh.normals.size() % 3 != 0) {
            std::cerr << "Warning: in " << filename << ", some normals are incorrect. The mesh won't be loaded." << std::endl;
             continue;
        }

        unsigned int nbTriangles = shape.mesh.indices.size() / 3;
        unsigned int nbVertices = shape.mesh.positions.size() / 3;
        unsigned int nbNormals = shape.mesh.normals.size() / 3;

        triangles.resize(nbTriangles);
        for (std::size_t iT = 0 ; iT < nbTriangles ; ++iT) {
            triangles[iT].x = shape.mesh.indices[3*iT + 0];
            triangles[iT].y = shape.mesh.indices[3*iT + 1];
            triangles[iT].z = shape.mesh.indices[3*iT + 2];
        }

        vertices.resize(nbVertices);
        for (std::size_t iV = 0 ; iV < nbVertices ; ++iV) {
            vertices[iV].x = shape.mesh.positions[3*iV + 0];
            vertices[iV].y = shape.mesh.positions[3*iV + 1];
            vertices[iV].z = shape.mesh.positions[3*iV + 2];
        }

        normals.resize(nbNormals);
        for (std::size_t iN = 0 ; iN < nbNormals ; ++iN) {
            normals[iN].x = shape.mesh.normals[3*iN + 0];
            normals[iN].y = shape.mesh.normals[3*iN + 1];
            normals[iN].z = shape.mesh.normals[3*iN + 2];
        }
    }

    return true;
}
