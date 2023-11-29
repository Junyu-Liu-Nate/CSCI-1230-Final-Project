#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <shapes/common.h>

class Face {
public:
    int v[3]; // indices for vertices
    int vn[3]; // indices for vertex normals
};

class Mesh {
public:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<Face> faces;

    Mesh() = default;

    std::vector<float> generateVertexData();
    void calculateNormals();
};

Mesh loadMesh(const std::string& filePath);
