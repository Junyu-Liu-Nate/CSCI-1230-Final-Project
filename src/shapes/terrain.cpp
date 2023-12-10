#include "terrain.h"

#include <cmath>
#include <iostream>
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

// Constructor
TerrainGenerator::TerrainGenerator()
{
    // Task 8: turn off wireframe shading
    //  m_wireshade = true; // STENCIL CODE
    m_wireshade = false; // TA SOLUTION

    // Define resolution of terrain generation
    m_resolution = 100;

    // Generate random vector lookup table
    m_lookupSize = 1024;
    m_randVecLookup.reserve(m_lookupSize);

    // Initialize random number generator
    std::srand(1230);

    // Populate random vector lookup table
    for (int i = 0; i < m_lookupSize; i++)
    {
        m_randVecLookup.push_back(glm::vec2(std::rand() * 2.0 / RAND_MAX - 1.0,
                                            std::rand() * 2.0 / RAND_MAX - 1.0));
    }
}

// Destructor
TerrainGenerator::~TerrainGenerator()
{
    m_randVecLookup.clear();
}

// Helper for generateTerrain()
void addPointToVector(glm::vec3 point, std::vector<float>& vector) {
    vector.push_back(point.x);
    vector.push_back(point.y);
    vector.push_back(point.z);
}

void addPointToVectorVec2(glm::vec2 point, std::vector<float>& vector) {
    vector.push_back(point.x);
    vector.push_back(point.y);
}

// Generates the geometry of the output triangle mesh
std::vector<float> TerrainGenerator::generateTerrain(QString path, int bump) {
    std::vector<float> verts;
    verts.reserve(m_resolution * m_resolution * 6);

    // Load heightmap image
    isLoaded = heightmapImage.load(path);
    if (!isLoaded) {
        return verts;
    }

    for(int x = 0; x < m_resolution; x++) {
        for(int y = 0; y < m_resolution; y++) {
            int x1 = x;
            int y1 = y;

            int x2 = x + 1;
            int y2 = y + 1;

//            glm::vec3 p1 = getPosition(x1,y1);
//            glm::vec3 p2 = getPosition(x2,y1);
//            glm::vec3 p3 = getPosition(x2,y2);
//            glm::vec3 p4 = getPosition(x1,y2);

//            glm::vec3 n1 = getNormal(x1,y1);
//            glm::vec3 n2 = getNormal(x2,y1);
//            glm::vec3 n3 = getNormal(x2,y2);
//            glm::vec3 n4 = getNormal(x1,y2);

            // Currently simply use a rotation matrix to rotate so the height of mountains are along y direction
            glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1, 0, 0));

            // Apply this rotation to vertices
            glm::vec3 p1 = glm::vec3(rotationMatrix * glm::vec4(getPosition(x1, y1, bump), 1.0));
            glm::vec3 p2 = glm::vec3(rotationMatrix * glm::vec4(getPosition(x2, y1, bump), 1.0));
            glm::vec3 p3 = glm::vec3(rotationMatrix * glm::vec4(getPosition(x2, y2, bump), 1.0));
            glm::vec3 p4 = glm::vec3(rotationMatrix * glm::vec4(getPosition(x1, y2, bump), 1.0));

            // Apply this rotation to normals
            glm::vec3 n1 = glm::vec3(rotationMatrix * glm::vec4(getNormal(x1, y1, bump), 0.0));
            glm::vec3 n2 = glm::vec3(rotationMatrix * glm::vec4(getNormal(x2, y1, bump), 0.0));
            glm::vec3 n3 = glm::vec3(rotationMatrix * glm::vec4(getNormal(x2, y2, bump), 0.0));
            glm::vec3 n4 = glm::vec3(rotationMatrix * glm::vec4(getNormal(x1, y2, bump), 0.0));

//            p1.x -= 0.5;
            p1.z += 1.0;
//            p2.x -= 0.5;
            p2.z += 1.0;
//            p3.x -= 0.5;
            p3.z += 1.0;
//            p4.x -= 0.5;
            p4.z += 1.0;

            // tris 1
            // x1y1z1
            // x2y1z2
            // x2y2z3
            addPointToVector(p1, verts);
            addPointToVector(n1, verts);
            addPointToVectorVec2(glm::vec2(p1.x,-p1.z), verts); // Not using get color anymore, this is texture uv

            addPointToVector(p2, verts);
            addPointToVector(n2, verts);
            addPointToVectorVec2(glm::vec2(p2.x,-p2.z), verts);

            addPointToVector(p3, verts);
            addPointToVector(n3, verts);
            addPointToVectorVec2(glm::vec2(p3.x,-p3.z), verts);

            // tris 2
            // x1y1z1
            // x2y2z3
            // x1y2z4
            addPointToVector(p1, verts);
            addPointToVector(n1, verts);
            addPointToVectorVec2(glm::vec2(p1.x,-p1.z), verts);

            addPointToVector(p3, verts);
            addPointToVector(n3, verts);
            addPointToVectorVec2(glm::vec2(p3.x,-p3.z), verts);

            addPointToVector(p4, verts);
            addPointToVector(n4, verts);
            addPointToVectorVec2(glm::vec2(p4.x,-p4.z), verts);
        }
    }
    return verts;
}

// Samples the (infinite) random vector grid at (row, col)
glm::vec2 TerrainGenerator::sampleRandomVector(int row, int col)
{
    std::hash<int> intHash;
    int index = intHash(row * 41 + col * 43) % m_lookupSize;
    return m_randVecLookup.at(index);
}

// Takes a grid coordinate (row, col), [0, m_resolution), which describes a vertex in a plane mesh
// Returns a normalized position (x, y, z); x and y in range from [0, 1), and z is obtained from getHeight()
glm::vec3 TerrainGenerator::getPosition(int row, int col, int bump) {
    // Normalizing the planar coordinates to a unit square
    // makes scaling independent of sampling resolution.
    float x = 1.0 * row / m_resolution;
    float y = 1.0 * col / m_resolution;
    float z = getHeight(x, y, bump);
    return glm::vec3(x,y,z);
}

// ================== Students, please focus on the code below this point

// Helper for computePerlin() and, possibly, getColor()
float interpolate(float A, float B, float alpha) {
    // Task 4: implement your easing/interpolation function below
    float ease = 3 * std::pow(alpha, 2) - 2 * std::pow(alpha, 3);
    return A + ease * (B - A);

    // Return 0 as placeholder
    //    return 0;
}

// Takes a normalized (x, y) position, in range [0,1)
// Returns a height value by sampling a heightmap
float TerrainGenerator::getHeight(float x, float y, int bump) {
    if (isLoaded) {
        if (x < 0 || y < 0 || x >= 1 || y >= 1) {
            return 0.f;
        }

        int i = heightmapImage.size().width() * x;
        int j = heightmapImage.size().height() * y;

        float height = qGray(heightmapImage.pixel(i, j));
        float z = 0;
        int factor = 8;

        for (int n = 0; n < bump; n++) {
            z += computePerlin(x * factor, y * factor) / factor;
            factor *= 2;
        }

        return height / 1600.f + z; // divided by 800 instead of 255 so that the terrain appears smoother
    }
    else {
        return 0.0f;
    }
}

// Computes the normal of a vertex by averaging neighbors
glm::vec3 TerrainGenerator::getNormal(int row, int col, int bump) {
    // Task 9: Compute the average normal for the given input indices
    std::vector<glm::vec3> nieghborVertices;
    nieghborVertices.push_back(getPosition(row - 1, col - 1, bump));
    nieghborVertices.push_back(getPosition(row, col - 1, bump));
    nieghborVertices.push_back(getPosition(row + 1, col - 1, bump));
    nieghborVertices.push_back(getPosition(row + 1, col, bump));
    nieghborVertices.push_back(getPosition(row + 1, col + 1, bump));
    nieghborVertices.push_back(getPosition(row, col + 1, bump));
    nieghborVertices.push_back(getPosition(row - 1, col + 1, bump));
    nieghborVertices.push_back(getPosition(row - 1, col, bump));

    glm::vec3 vPosition = getPosition(row, col, bump);

    std::vector<glm::vec3> normals;
    for (int i = 0; i < 8; i++) {
        glm::vec3 vn_a = nieghborVertices.at(i) - vPosition;
        glm::vec3 vn_b;
        if (i+1 < 8)
            vn_b = nieghborVertices.at(i+1) - vPosition;
        else
            vn_b = nieghborVertices.at(0) - vPosition;
        normals.push_back(glm::cross(vn_a, vn_b));
    }

    glm::vec3 avgNormal = glm::vec3(0.0f, 0.0f, 0.0f);
    for (glm::vec3 normal : normals) {
        avgNormal += normal;
    }

    return glm::normalize(avgNormal);
}

// Computes color of vertex using normal and, optionally, position
glm::vec3 TerrainGenerator::getColor(glm::vec3 normal, glm::vec3 position) {
    // Task 10: compute color as a function of the normal and position
    //    if (position.z > 0.06) {
    //        return glm::vec3(1,1,1);
    //    }
    //    else {
    //        if (glm::dot(normal, glm::vec3(0,0,1)) > 0.9) {
    //            return glm::vec3(1,1,1);
    //        }
    //        else {
    //            return glm::vec3(0.5,0.5,0.5);
    //        }
    //    }

    float easeZ = std::pow(position.z, 2) - 2 * std::pow(position.z, 3) + 0.05;
    float heightValue = easeZ*12.0f;

    float angle = glm::dot(normal, glm::vec3(0,0,1));
    //    angle = glm::clamp(angle, 0.0f, 1.0f);
    //    float easeNormal = std::pow(angle, 2) - 2 * std::pow(angle, 3);
    float easeNormal = 0.5 * angle * angle - 0.5 * angle;
    float normalValue = easeNormal*0.5f;

    float colorValue = heightValue + normalValue;
    colorValue = glm::clamp(colorValue, 0.0f, 1.0f);

    return glm::vec3(colorValue, colorValue, colorValue);

    // Return white as placeholder
    //    return glm::vec3(1,1,1);
}

// Computes the intensity of Perlin noise at some point
float TerrainGenerator::computePerlin(float x, float y) {
    // Task 1: get grid indices (as ints)
    int xFloor = std::floor(x);
    int yFloor = std::floor(y);

    // Task 2: compute offset vectors
    glm::vec2 vectorTL = glm::vec2(x, y) - glm::vec2(xFloor, yFloor);
    glm::vec2 vectorTR = glm::vec2(x, y) - glm::vec2(xFloor + 1, yFloor);
    glm::vec2 vectorBR = glm::vec2(x, y) - glm::vec2(xFloor + 1, yFloor + 1);
    glm::vec2 vectorBL = glm::vec2(x, y) - glm::vec2(xFloor, yFloor + 1);

    // Task 3: compute the dot product between the grid point direction vectors and its offset vectors
    float A = glm::dot(sampleRandomVector(yFloor, xFloor), vectorTL); // dot product between top-left direction and its offset
    float B = glm::dot(sampleRandomVector(yFloor, xFloor + 1), vectorTR); // dot product between top-right direction and its offset
    float C = glm::dot(sampleRandomVector(yFloor + 1, xFloor + 1), vectorBR); // dot product between bottom-right direction and its offset
    float D = glm::dot(sampleRandomVector(yFloor + 1, xFloor), vectorBL); // dot product between bottom-left direction and its offset

    // Task 5: Debug this line to properly use your interpolation function to produce the correct value
    //    return interpolate(interpolate(A, B, 0.5), interpolate(D, C, 0.5), 0.5);
    return interpolate(interpolate(A, B, x-xFloor), interpolate(D, C, x-xFloor), y-yFloor);

    // Return 0 as a placeholder
    return 0;
}
