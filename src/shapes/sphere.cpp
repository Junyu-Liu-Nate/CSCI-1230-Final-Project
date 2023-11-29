#include "Sphere.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

void Sphere::updateParams(int param1, int param2, bool isTexture, float repeatU, float repeatV, QString imgPath) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    this->isTexture = isTexture;
    this->repeatU = repeatU;
    this->repeatV = repeatV;
    this->imgPath = imgPath;
    setVertexData();
}

void Sphere::makeTile(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight,
                      glm::vec2 topLeftTexture,
                      glm::vec2 topRightTexture,
                      glm::vec2 bottomLeftTexture,
                      glm::vec2 bottomRightTexture) {
    // First triangle
    insertVec3(m_vertexData, topLeft);
    glm::vec3 topLeftNormal = glm::normalize(topLeft);
    insertVec3(m_vertexData, topLeftNormal);
    insertVec2(m_vertexData, topLeftTexture);

    insertVec3(m_vertexData, bottomLeft);
    glm::vec3 bottomLeftNormal = glm::normalize(bottomLeft);
    insertVec3(m_vertexData, bottomLeftNormal);
    insertVec2(m_vertexData, bottomLeftTexture);

    insertVec3(m_vertexData, bottomRight);
    glm::vec3 bottomRightNormal = glm::normalize(bottomRight);
    insertVec3(m_vertexData, bottomRightNormal);
    insertVec2(m_vertexData, bottomRightTexture);

    // Second triangle
    insertVec3(m_vertexData, topLeft);
    glm::vec3 topLeftNormal2 = glm::normalize(topLeft);
    insertVec3(m_vertexData, topLeftNormal2);
    insertVec2(m_vertexData, topLeftTexture);

    insertVec3(m_vertexData, bottomRight);
    glm::vec3 bottomRightNormal2 = glm::normalize(bottomRight);
    insertVec3(m_vertexData, bottomRightNormal2);
    insertVec2(m_vertexData, bottomRightTexture);

    insertVec3(m_vertexData, topRight);
    glm::vec3 topRightNormal = glm::normalize(topRight);
    insertVec3(m_vertexData, topRightNormal);
    insertVec2(m_vertexData, topRightTexture);
}

void Sphere::makeWedge(float currentTheta, float nextTheta) {
    float phiStep = glm::radians(180.f / m_param1);
    float currentPhi;
    float nextPhi;
    for (int i = 0; i < m_param1; i++) {
        currentPhi = i * phiStep;
        nextPhi = (i + 1) * phiStep;

        glm::vec3 topLeft = {m_radius * glm::sin(currentPhi) * glm::sin(currentTheta), m_radius * glm::cos(currentPhi), m_radius * glm::sin(currentPhi) * glm::cos(currentTheta)};
        glm::vec3 topRight = {m_radius * glm::sin(currentPhi) * glm::sin(nextTheta), m_radius * glm::cos(currentPhi), m_radius * glm::sin(currentPhi) * glm::cos(nextTheta)};
        glm::vec3 bottomLeft = {m_radius * glm::sin(nextPhi) * glm::sin(currentTheta), m_radius * glm::cos(nextPhi), m_radius * glm::sin(nextPhi) * glm::cos(currentTheta)};
        glm::vec3 bottomRight = {m_radius * glm::sin(nextPhi) * glm::sin(nextTheta), m_radius * glm::cos(nextPhi), m_radius * glm::sin(nextPhi) * glm::cos(nextTheta)};

        std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeft, topRight, bottomLeft, bottomRight);
        glm::vec2 topLeftTexture = tileTextureUV.at(0);
        glm::vec2 topRightTexture = tileTextureUV.at(1);
        glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
        glm::vec2 bottomRightTexture = tileTextureUV.at(3);

        makeTile(topLeft, topRight, bottomLeft, bottomRight, topLeftTexture, topRightTexture, bottomLeftTexture, bottomRightTexture);
    }
}

// Compute texture UV coordinates
std::vector<glm::vec2> Sphere::calculateTileTextureUV(glm::vec3 topLeft,
                                                      glm::vec3 topRight,
                                                      glm::vec3 bottomLeft,
                                                      glm::vec3 bottomRight) {
    std::vector<glm::vec2> tileTextureUV;

    glm::vec2 topLeftTexture;
    if (isTexture) {
        topLeftTexture = sphereTexture(topLeft, repeatU, repeatV);
    }
    else {
        topLeftTexture = {0,0};
    }
    tileTextureUV.push_back(topLeftTexture);

    glm::vec2 topRightTexture;
    if (isTexture) {
        topRightTexture = sphereTexture(topRight, repeatU, repeatV);
    }
    else {
        topRightTexture = {0,0};
    }
    tileTextureUV.push_back(topRightTexture);

    glm::vec2 bottomLeftTexture;
    if (isTexture) {
        bottomLeftTexture = sphereTexture(bottomLeft, repeatU, repeatV);
    }
    else {
        bottomLeftTexture = {0,0};
    }
    tileTextureUV.push_back(bottomLeftTexture);

    glm::vec2 bottomRightTexture;
    if (isTexture) {
        bottomRightTexture = sphereTexture(bottomRight, repeatU, repeatV);
    }
    else {
        bottomRightTexture = {0,0};
    }
    tileTextureUV.push_back(bottomRightTexture);

    return tileTextureUV;
}

glm::vec2 Sphere::sphereTexture(glm::vec3 point, float repeatU, float repeatV) {
    float x = point.x;
    float y = point.y;
    float z = point.z;

    float u;
    float theta = atan2(z, x);
    if (theta < 0) {
        u = -theta / (2 * M_PI);
    } else {
        u = 1 - theta / (2 * M_PI);
    }

    float v = asin(y / 0.5) / M_PI + 0.5;

    int segment_u = static_cast<int>(u * repeatU);
    float u_prime = u * repeatU - segment_u;

    int segment_v = static_cast<int>(v * repeatV);
    float v_prime = v * repeatV - segment_v;

    return glm::vec2(u_prime, v_prime);
}

void Sphere::makeSphere() {
    float thetaStep = glm::radians(360.f / m_param2);
    float currentTheta;
    float nextTheta;
    for (int i = 0; i < m_param2; i++) {
        currentTheta = i * thetaStep;
        nextTheta = (i + 1) * thetaStep;
        makeWedge(currentTheta, nextTheta);
    }
}

void Sphere::setVertexData() {
    makeSphere();
}
