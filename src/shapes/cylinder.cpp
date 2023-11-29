#include "Cylinder.h"

void Cylinder::updateParams(int param1, int param2, bool isTexture, float repeatU, float repeatV, QString imgPath) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    this->isTexture = isTexture;
    this->repeatU = repeatU;
    this->repeatV = repeatV;
    this->imgPath = imgPath;
    setVertexData();
}

void Cylinder::makeTile(glm::vec3 topLeft,
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

    insertVec3(m_vertexData, bottomRight);
    glm::vec3 bottomRightNormal = glm::normalize(glm::cross(glm::normalize(bottomRight - bottomLeft), glm::normalize(topLeft - bottomRight)));
    insertVec3(m_vertexData, bottomRightNormal);
    insertVec2(m_vertexData, bottomRightTexture);

    // Second triangle
    insertVec3(m_vertexData, topLeft);
    glm::vec3 topLeftNormal2 = glm::normalize(glm::cross(glm::normalize(topLeft - topRight), glm::normalize(bottomRight - topLeft)));
    insertVec3(m_vertexData, topLeftNormal2);
    insertVec2(m_vertexData, topLeftTexture);

    insertVec3(m_vertexData, bottomRight);
    glm::vec3 bottomRightNormal2 = glm::normalize(glm::cross(glm::normalize(bottomRight - topLeft), glm::normalize(topRight - bottomRight)));
    insertVec3(m_vertexData, bottomRightNormal2);
    insertVec2(m_vertexData, bottomRightTexture);

    insertVec3(m_vertexData, topRight);
    glm::vec3 topRightNormal = glm::normalize(glm::cross(glm::normalize(topRight - bottomRight), glm::normalize(topLeft - topRight)));
    insertVec3(m_vertexData, topRightNormal);
    insertVec2(m_vertexData, topRightTexture);
}

void Cylinder::makeTileSide(glm::vec3 topLeft,
                            glm::vec3 topRight,
                            glm::vec3 bottomLeft,
                            glm::vec3 bottomRight,
                            glm::vec2 topLeftTexture,
                            glm::vec2 topRightTexture,
                            glm::vec2 bottomLeftTexture,
                            glm::vec2 bottomRightTexture) {
    // Normals are calculated by pointing from the center along x-z plane
    // First triangle
    insertVec3(m_vertexData, topLeft);
    glm::vec3 topLeftNormal = glm::normalize(glm::vec3(topLeft.x, 0, topLeft.z));
    insertVec3(m_vertexData, topLeftNormal);
    insertVec2(m_vertexData, topLeftTexture);

    insertVec3(m_vertexData, bottomLeft);
    glm::vec3 bottomLeftNormal = glm::normalize(glm::vec3(bottomLeft.x, 0, bottomLeft.z));
    insertVec3(m_vertexData, bottomLeftNormal);
    insertVec2(m_vertexData, bottomLeftTexture);

    insertVec3(m_vertexData, bottomRight);
    glm::vec3 bottomRightNormal = glm::normalize(glm::vec3(bottomRight.x, 0, bottomRight.z));
    insertVec3(m_vertexData, bottomRightNormal);
    insertVec2(m_vertexData, bottomRightTexture);

    // Second triangle
    insertVec3(m_vertexData, topLeft);
    glm::vec3 topLeftNormal2 = glm::normalize(glm::vec3(topLeft.x, 0, topLeft.z));
    insertVec3(m_vertexData, topLeftNormal2);
    insertVec2(m_vertexData, topLeftTexture);

    insertVec3(m_vertexData, bottomRight);
    glm::vec3 bottomRightNormal2 = glm::normalize(glm::vec3(bottomRight.x, 0, bottomRight.z));
    insertVec3(m_vertexData, bottomRightNormal2);
    insertVec2(m_vertexData, bottomRightTexture);

    insertVec3(m_vertexData, topRight);
    glm::vec3 topRightNormal = glm::normalize(glm::vec3(topRight.x, 0, topRight.z));
    insertVec3(m_vertexData, topRightNormal);
    insertVec2(m_vertexData, topRightTexture);
}

void Cylinder::makeSide(float currentTheta, float nextTheta) {
    float yStep = 1.0 / m_param1;

    for (int i = 0; i < m_param1; i++) {
        float xLeft = m_radius * glm::sin(currentTheta);
        float zLeft = m_radius * glm::cos(currentTheta);
        float xRight = m_radius * glm::sin(nextTheta);
        float zRight = m_radius * glm::cos(nextTheta);

        glm::vec3 topLeft = {xLeft, 0.5 - i * yStep, zLeft};
        glm::vec3 topRight = {xRight, 0.5 - i * yStep, zRight};
        glm::vec3 bottomLeft = {xLeft, 0.5 - (i + 1) * yStep, zLeft};
        glm::vec3 bottomRight = {xRight, 0.5 - (i + 1) * yStep, zRight};

        std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeft, topRight, bottomLeft, bottomRight);
        glm::vec2 topLeftTexture = tileTextureUV.at(0);
        glm::vec2 topRightTexture = tileTextureUV.at(1);
        glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
        glm::vec2 bottomRightTexture = tileTextureUV.at(3);

        makeTileSide(topLeft, topRight, bottomLeft, bottomRight, topLeftTexture, topRightTexture, bottomLeftTexture, bottomRightTexture);
    }
}

void Cylinder::makeCap(float currentTheta, float nextTheta) {
    float step = m_radius / m_param1;

    // Top cap
    for (int i = 0; i < m_param1; i++) {
        glm::vec3 topLeft = {i * step * glm::sin(currentTheta), 0.5, i * step * glm::cos(currentTheta)};
        glm::vec3 topRight = {i * step * glm::sin(nextTheta), 0.5, i * step * glm::cos(nextTheta)};
        glm::vec3 bottomLeft = {(i + 1) * step * glm::sin(currentTheta), 0.5, (i + 1) * step * glm::cos(currentTheta)};
        glm::vec3 bottomRight = {(i + 1) * step * glm::sin(nextTheta), 0.5, (i + 1) * step * glm::cos(nextTheta)};

        std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeft, topRight, bottomLeft, bottomRight);
        glm::vec2 topLeftTexture = tileTextureUV.at(0);
        glm::vec2 topRightTexture = tileTextureUV.at(1);
        glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
        glm::vec2 bottomRightTexture = tileTextureUV.at(3);

        makeTile(topLeft, topRight, bottomLeft, bottomRight, topLeftTexture, topRightTexture, bottomLeftTexture, bottomRightTexture);
    }

    // Bottom cap
    for (int i = 0; i < m_param1; i++) {
        glm::vec3 topRight = {i * step * glm::sin(currentTheta), -0.5, i * step * glm::cos(currentTheta)};
        glm::vec3 topLeft = {i * step * glm::sin(nextTheta), -0.5, i * step * glm::cos(nextTheta)};
        glm::vec3 bottomRight = {(i + 1) * step * glm::sin(currentTheta), -0.5, (i + 1) * step * glm::cos(currentTheta)};
        glm::vec3 bottomLeft = {(i + 1) * step * glm::sin(nextTheta), -0.5, (i + 1) * step * glm::cos(nextTheta)};

        std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeft, topRight, bottomLeft, bottomRight);
        glm::vec2 topLeftTexture = tileTextureUV.at(0);
        glm::vec2 topRightTexture = tileTextureUV.at(1);
        glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
        glm::vec2 bottomRightTexture = tileTextureUV.at(3);

        makeTile(topLeft, topRight, bottomLeft, bottomRight, topLeftTexture, topRightTexture, bottomLeftTexture, bottomRightTexture);
    }
}

std::vector<glm::vec2> Cylinder::calculateTileTextureUV(glm::vec3 topLeft,
                                                      glm::vec3 topRight,
                                                      glm::vec3 bottomLeft,
                                                      glm::vec3 bottomRight) {
    std::vector<glm::vec2> tileTextureUV;

    glm::vec2 topLeftTexture;
    if (isTexture) {
        topLeftTexture = cylinderTexture(topLeft, repeatU, repeatV);
    }
    else {
        topLeftTexture = {0,0};
    }
    tileTextureUV.push_back(topLeftTexture);

    glm::vec2 topRightTexture;
    if (isTexture) {
        topRightTexture = cylinderTexture(topRight, repeatU, repeatV);
    }
    else {
        topRightTexture = {0,0};
    }
    tileTextureUV.push_back(topRightTexture);

    glm::vec2 bottomLeftTexture;
    if (isTexture) {
        bottomLeftTexture = cylinderTexture(bottomLeft, repeatU, repeatV);
    }
    else {
        bottomLeftTexture = {0,0};
    }
    tileTextureUV.push_back(bottomLeftTexture);

    glm::vec2 bottomRightTexture;
    if (isTexture) {
        bottomRightTexture = cylinderTexture(bottomRight, repeatU, repeatV);
    }
    else {
        bottomRightTexture = {0,0};
    }
    tileTextureUV.push_back(bottomRightTexture);

    return tileTextureUV;
}

glm::vec2 Cylinder::cylinderTexture(glm::vec3 point, float repeatU, float repeatV) {
    float x = point.x;
    float y = point.y;
    float z = point.z;

    float u, v;

    float epsilon = 1e-5;  // Added epsilon for floating-point precision issues

    if (std::abs(y + 0.5) < epsilon || std::abs(y - 0.5) < epsilon) {  // Adjusted for precision
        if (std::abs(y + 0.5) < epsilon) {
            u = x + 0.5;
            v = z + 0.5;
        } else {
            u = x + 0.5;
            v = -z + 0.5;
        }
    } else {  // Side of the cylinder
        float theta = atan2(z, x);
        if (theta < 0) {
            u = -theta / (2 * M_PI);
        } else {
            u = 1 - theta / (2 * M_PI);
        }

        v = y + 0.5;  // Maps [-1,1] to [0,1]
    }

    int segment_u = static_cast<int>(u * repeatU);
    float u_prime = u * repeatU - segment_u;

    int segment_v = static_cast<int>(v * repeatV);
    float v_prime = v * repeatV - segment_v;

    return glm::vec2(u_prime, v_prime);
}

void Cylinder::makeCylinder() {
    float thetaStep = glm::radians(360.f / m_param2);
    float currentTheta;
    float nextTheta;
    for (int i = 0; i < m_param2; i++) {
        currentTheta = i * thetaStep;
        nextTheta = (i + 1) * thetaStep;
        makeSide(currentTheta, nextTheta);
        makeCap(currentTheta, nextTheta);
    }
}

void Cylinder::setVertexData() {
    makeCylinder();
}
