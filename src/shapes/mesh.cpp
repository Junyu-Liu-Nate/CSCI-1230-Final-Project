#include "mesh.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>>

// Note: error handling added for invalid mesh file
Mesh loadMesh(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open .obj file");
    }

    Mesh mesh;
    std::vector<glm::vec3> tempNormals;
    std::vector<int> normalCount;
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream stream(line);
        std::string type;
        stream >> type;

        if (type == "v") {
            glm::vec3 vertex;
            stream >> vertex.x >> vertex.y >> vertex.z;
            if (stream.fail()) {
                throw std::runtime_error("Error reading vertex data.");
            }
            mesh.vertices.push_back(vertex);
            tempNormals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
            normalCount.push_back(0);
        }
        else if (type == "vn") {
            glm::vec3 normal;
            stream >> normal.x >> normal.y >> normal.z;
            if (stream.fail()) {
                throw std::runtime_error("Error reading vertex normal data.");
            }
            mesh.normals.push_back(normal);
        }
        else if (type == "f") {
            Face face;
            char skip;
            std::fill_n(face.vn, 3, -1); // Initialize normal indices to -1

            for (int i = 0; i < 3; i++) {
                stream >> face.v[i]; // Read vertex index
                face.v[i] -= 1; // Adjust for 0-based indexing

                if (stream.peek() == '/') {
                    stream >> skip; // Skip the '/'
                    if (stream.peek() == '/') {
                        stream >> skip; // Skip the second '/'
                        // Read normal index, if available
                        stream >> face.vn[i];
                        face.vn[i] -= 1; // Adjust for 0-based indexing
                    }
                }
            }
            mesh.faces.push_back(face);
        }

        // Skipping comment lines
        else if (type == "#") {
            continue;
        }
    }

    file.close();

    // If no normals were provided in the file, calculate them
    if (mesh.normals.empty()) {
        mesh.calculateNormals();
    }

    return mesh;
}

void Mesh::calculateNormals() {
    // Clear the normals if they were already calculated
    normals.clear();

    // Temporary normals to store the calculated face normals
    std::vector<glm::vec3> tempFaceNormals;
    tempFaceNormals.reserve(faces.size());

    for (const auto& face : faces) {
        // Calculate the face normal
        glm::vec3 v1 = vertices[face.v[1]] - vertices[face.v[0]];
        glm::vec3 v2 = vertices[face.v[2]] - vertices[face.v[0]];
        glm::vec3 normal = glm::normalize(glm::cross(v1, v2));
        tempFaceNormals.push_back(normal);
    }

    // Update the normal indices
    for (size_t i = 0; i < faces.size(); ++i) {
        // Push back the same normal for all three vertices of the face
        normals.push_back(tempFaceNormals[i]);
        normals.push_back(tempFaceNormals[i]);
        normals.push_back(tempFaceNormals[i]);

        // Update the face's normal indices
        faces[i].vn[0] = i * 3;
        faces[i].vn[1] = i * 3 + 1;
        faces[i].vn[2] = i * 3 + 2;
    }
}

std::vector<float> Mesh::generateVertexData() {
    std::vector<float> vertexData;
    for (const auto& face : faces) {
        for (int i = 0; i < 3; i++) {
            insertVec3(vertexData, vertices[face.v[i]]);
            insertVec3(vertexData, normals[face.vn[i]]);
            insertVec2(vertexData, glm::vec2(0,0));
        }
    }
    return vertexData;
}


