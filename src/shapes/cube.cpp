#include "cube.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

void Cube::updateParams(int param1, bool isTexture, float repeatU, float repeatV, QString imgPath) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    this->isTexture = isTexture;
    this->repeatU = repeatU;
    this->repeatV = repeatV;
    this->imgPath = imgPath;
    setVertexData();
}


void Cube::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight,
                    glm::vec2 topLeftTexture,
                    glm::vec2 topRightTexture,
                    glm::vec2 bottomLeftTexture,
                    glm::vec2 bottomRightTexture) {
    // First triangle
    insertVec3(m_vertexData, topLeft);
    glm::vec3 topLeftNormal = glm::normalize(glm::cross(glm::normalize(topLeft - bottomRight), glm::normalize(bottomLeft - topLeft)));
    insertVec3(m_vertexData, topLeftNormal);
    insertVec2(m_vertexData, topLeftTexture);

    insertVec3(m_vertexData, bottomLeft);
    glm::vec3 bottomLeftNormal = glm::normalize(glm::cross(glm::normalize(bottomLeft- topLeft), glm::normalize(bottomRight - bottomLeft)));
    insertVec3(m_vertexData, bottomLeftNormal);
    insertVec2(m_vertexData, bottomLeftTexture);

    insertVec3(m_vertexData, topRight);
    glm::vec3 topRightNormal = glm::normalize(glm::cross(glm::normalize(topRight - bottomRight), glm::normalize(topLeft - topRight)));
    insertVec3(m_vertexData, topRightNormal);
    insertVec2(m_vertexData, topRightTexture);

    // Second triangle
    insertVec3(m_vertexData, topRight);
    glm::vec3 topRightNormal2 = glm::normalize(glm::cross(glm::normalize(topRight - bottomRight), glm::normalize(topLeft - topRight)));
    insertVec3(m_vertexData, topRightNormal2);
    insertVec2(m_vertexData, topRightTexture);

    insertVec3(m_vertexData, bottomLeft);
    glm::vec3 bottomLeftNormal2 = glm::normalize(glm::cross(glm::normalize(bottomLeft- topLeft), glm::normalize(bottomRight - bottomLeft)));
    insertVec3(m_vertexData, bottomLeftNormal2);
    insertVec2(m_vertexData, bottomLeftTexture);

    insertVec3(m_vertexData, bottomRight);
    glm::vec3 bottomRightNormal = glm::normalize(glm::cross(glm::normalize(bottomRight - topLeft), glm::normalize(topRight - bottomRight)));
    insertVec3(m_vertexData, bottomRightNormal);
    insertVec2(m_vertexData, bottomRightTexture);
}

void Cube::makeFace(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight,
                    int faceIndex) {
    float stepU = 1.0 /  m_param1;
    float stepV = 1.0 /  m_param1;

    if (topLeft.z == topRight.z && topRight.z == bottomLeft.z && bottomLeft.z == bottomRight.z) {
        if (bottomLeft.x < bottomRight.x) {
            for (int i = 0; i < m_param1; i++) {
                for (int j = 0; j < m_param1; j++) {
                    glm::vec3 bottomLeftTile = glm::vec3(-0.5 + stepU*i, -0.5 + stepV*j, topLeft.z);
                    glm::vec3 topLeftTile = bottomLeftTile + glm::vec3(0, stepV, 0);
                    glm::vec3 bottomRightTile = bottomLeftTile + glm::vec3(stepU, 0, 0);
                    glm::vec3 topRightTile = bottomLeftTile + glm::vec3(stepU, stepV, 0);

                    std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile);
                    glm::vec2 topLeftTexture = tileTextureUV.at(0);
                    glm::vec2 topRightTexture = tileTextureUV.at(1);
                    glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
                    glm::vec2 bottomRightTexture = tileTextureUV.at(3);

                    makeTile(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile, topLeftTexture, topRightTexture, bottomLeftTexture, bottomRightTexture);
                }
            }
        }
        else {
            for (int i = 0; i < m_param1; i++) {
                for (int j = 0; j < m_param1; j++) {
                    glm::vec3 bottomRightTile = glm::vec3(-0.5 + stepU*i, -0.5 + stepV*j, topLeft.z);
                    glm::vec3 topLeftTile = bottomRightTile + glm::vec3(stepU, stepV, 0);
                    glm::vec3 bottomLeftTile = bottomRightTile + glm::vec3(stepU, 0, 0);
                    glm::vec3 topRightTile = bottomRightTile + glm::vec3(0, stepV, 0);

                    std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile);
                    glm::vec2 topLeftTexture = tileTextureUV.at(0);
                    glm::vec2 topRightTexture = tileTextureUV.at(1);
                    glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
                    glm::vec2 bottomRightTexture = tileTextureUV.at(3);

                    makeTile(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile, topLeftTexture, topRightTexture, bottomLeftTexture, bottomRightTexture);
                }
            }
        }
    }

    if (topLeft.x == topRight.x && topRight.x == bottomLeft.x && bottomLeft.x == bottomRight.x) {
        if (bottomLeft.y < bottomRight.y) {
            for (int i = 0; i < m_param1; i++) {
                for (int j = 0; j < m_param1; j++) {
                    glm::vec3 bottomLeftTile = glm::vec3(topLeft.x, -0.5 + stepV * i, -0.5 + stepU * j);
                    glm::vec3 topLeftTile = bottomLeftTile + glm::vec3(0, 0, stepV);
                    glm::vec3 bottomRightTile = bottomLeftTile + glm::vec3(0, stepU, 0);
                    glm::vec3 topRightTile = bottomLeftTile + glm::vec3(0, stepV, stepU);

                    std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile);
                    glm::vec2 topLeftTexture = tileTextureUV.at(0);
                    glm::vec2 topRightTexture = tileTextureUV.at(1);
                    glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
                    glm::vec2 bottomRightTexture = tileTextureUV.at(3);

                    makeTile(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile, topLeftTexture, topRightTexture, bottomLeftTexture, bottomRightTexture);
                }
            }
        }
        else {
            for (int i = 0; i < m_param1; i++) {
                for (int j = 0; j < m_param1; j++) {
                    glm::vec3 bottomRightTile = glm::vec3(topLeft.x, -0.5 + stepV * i, -0.5 + stepU * j);
                    glm::vec3 topRightTile = bottomRightTile + glm::vec3(0, 0, stepV);
                    glm::vec3 bottomLeftTile = bottomRightTile + glm::vec3(0, stepU, 0);
                    glm::vec3 topLeftTile = bottomRightTile + glm::vec3(0, stepU, stepV);

                    std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile);
                    glm::vec2 topLeftTexture = tileTextureUV.at(0);
                    glm::vec2 topRightTexture = tileTextureUV.at(1);
                    glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
                    glm::vec2 bottomRightTexture = tileTextureUV.at(3);

                    makeTile(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile, topLeftTexture, topRightTexture, bottomLeftTexture, bottomRightTexture);
                }
            }
        }
    }

    if (topLeft.y == topRight.y && topRight.y == bottomLeft.y && bottomLeft.y == bottomRight.y) {
        if (bottomLeft.x < bottomRight.x) {
            for (int i = 0; i < m_param1; i++) {
                for (int j = 0; j < m_param1; j++) {
                    glm::vec3 bottomLeftTile = glm::vec3(-0.5 + stepU * i, -topLeft.y, -0.5 + stepV * j);
                    glm::vec3 topLeftTile = bottomLeftTile + glm::vec3(0, 0, stepV);
                    glm::vec3 bottomRightTile = bottomLeftTile + glm::vec3(stepU, 0, 0);
                    glm::vec3 topRightTile = bottomLeftTile + glm::vec3(stepU, 0, stepV);

                    std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile);
                    glm::vec2 topLeftTexture = tileTextureUV.at(0);
                    glm::vec2 topRightTexture = tileTextureUV.at(1);
                    glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
                    glm::vec2 bottomRightTexture = tileTextureUV.at(3);

                    makeTile(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile, topLeftTexture, topRightTexture, bottomLeftTexture, bottomRightTexture);
                }
            }
        }
        else {
            for (int i = 0; i < m_param1; i++) {
                for (int j = 0; j < m_param1; j++) {
                    glm::vec3 bottomRightTile = glm::vec3(-0.5 + stepU * i, -topLeft.y, -0.5 + stepV * j);
                    glm::vec3 topRightTile = bottomRightTile + glm::vec3(0, 0, stepV);
                    glm::vec3 bottomLeftTile = bottomRightTile + glm::vec3(stepU, 0, 0);
                    glm::vec3 topLeftTile = bottomRightTile + glm::vec3(stepU, 0, stepV);

                    std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile);
                    glm::vec2 topLeftTexture = tileTextureUV.at(0);
                    glm::vec2 topRightTexture = tileTextureUV.at(1);
                    glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
                    glm::vec2 bottomRightTexture = tileTextureUV.at(3);

                    makeTile(topLeftTile, topRightTile, bottomLeftTile, bottomRightTile, topLeftTexture, topRightTexture, bottomLeftTexture, bottomRightTexture);
                }
            }
        }
    }
}

std::vector<glm::vec2> Cube::calculateTileTextureUV(glm::vec3 topLeft,
                                                    glm::vec3 topRight,
                                                    glm::vec3 bottomLeft,
                                                    glm::vec3 bottomRight) {
    std::vector<glm::vec2> tileTextureUV;

    glm::vec2 topLeftTexture;
    if (isTexture) {
        topLeftTexture = cubeTexture(topLeft, repeatU, repeatV);
    }
    else {
        topLeftTexture = {0,0};
    }
    tileTextureUV.push_back(topLeftTexture);

    glm::vec2 topRightTexture;
    if (isTexture) {
        topRightTexture = cubeTexture(topRight, repeatU, repeatV);
    }
    else {
        topRightTexture = {0,0};
    }
    tileTextureUV.push_back(topRightTexture);

    glm::vec2 bottomLeftTexture;
    if (isTexture) {
        bottomLeftTexture = cubeTexture(bottomLeft, repeatU, repeatV);
    }
    else {
        bottomLeftTexture = {0,0};
    }
    tileTextureUV.push_back(bottomLeftTexture);

    glm::vec2 bottomRightTexture;
    if (isTexture) {
        bottomRightTexture = cubeTexture(bottomRight, repeatU, repeatV);
    }
    else {
        bottomRightTexture = {0,0};
    }
    tileTextureUV.push_back(bottomRightTexture);

    return tileTextureUV;
}

glm::vec2 Cube::cubeTexture(glm::vec3 point, float repeatU, float repeatV) {
    const float epsilon = 1e-5f;  // Epsilon for floating point comparisons

    float x = point.x;
    float y = point.y;
    float z = point.z;

    float u, v;

    // Identify the face that's hit and compute u, v coordinates
    if (std::abs(x - 0.5) < epsilon) {
        u = (-z + 0.5);
        v = (y + 0.5);
    }
    else if (std::abs(x + 0.5) < epsilon) {
        u = (z + 0.5);
        v = (y + 0.5);
    }
    else if (std::abs(y - 0.5) < epsilon) {
        u = (x + 0.5);
        v = (-z + 0.5);
    }
    else if (std::abs(y + 0.5) < epsilon) {
        u = (x + 0.5);
        v = (z + 0.5);
    }
    else if (std::abs(z - 0.5) < epsilon) {
        u = (x + 0.5);
        v = (y + 0.5);
    }
    else {  // z + 0.5
        u = (-x + 0.5);
        v = (y + 0.5);
    }

    int segment_u = static_cast<int>(u * repeatU);
    float u_prime = u * repeatU - segment_u;

    int segment_v = static_cast<int>(v * repeatV);
    float v_prime = v * repeatV - segment_v;

    return glm::vec2(u_prime, v_prime);
}


void Cube::setVertexData() {
    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3( 0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, -0.5f, 0.5f),
             glm::vec3( 0.5f, -0.5f, 0.5f),
             0);

    makeFace(glm::vec3(0.5f,  0.5f, -0.5f),
             glm::vec3(-0.5f,  0.5f, -0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             1);

    makeFace(glm::vec3(0.5f,  -0.5f, 0.5f),
             glm::vec3(0.5f,  0.5f, 0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f),
             glm::vec3(0.5f, 0.5f, -0.5f),
             2);

    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f,  -0.5f, 0.5f),
             glm::vec3(-0.5f, 0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             3);

    makeFace(glm::vec3(-0.5f,  0.5f, 0.5f),
             glm::vec3(0.5f,  0.5f, 0.5f),
             glm::vec3(-0.5f, 0.5f, -0.5f),
             glm::vec3(0.5f, 0.5f, -0.5f),
             4);

    makeFace(glm::vec3(0.5f,  -0.5f, 0.5f),
             glm::vec3(-0.5f,  -0.5f, 0.5f),
             glm::vec3(0.5f, -0.5f, -0.5f),
             glm::vec3(-0.5f, -0.5f, -0.5f),
             5);
}
