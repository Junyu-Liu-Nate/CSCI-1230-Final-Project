#include "Cone.h"

void Cone::updateParams(int param1, int param2, bool isTexture, float repeatU, float repeatV, QString imgPath) {
    m_vertexData = std::vector<float>();
    m_param1 = param1;
    m_param2 = param2;
    this->isTexture = isTexture;
    this->repeatU = repeatU;
    this->repeatV = repeatV;
    this->imgPath = imgPath;
    setVertexData();
}

void Cone::makeTile(glm::vec3 topLeft,
                    glm::vec3 topRight,
                    glm::vec3 bottomLeft,
                    glm::vec3 bottomRight,
                    glm::vec2 topLeftTexture,
                    glm::vec2 topRightTexture,
                    glm::vec2 bottomLeftTexture,
                    glm::vec2 bottomRightTexture) {
    // First triangle
    insertVec3(m_vertexData, topLeft);
    glm::vec3 topLeftNormal = glm::normalize(glm::cross(glm::normalize(topLeft - bottomLeft), glm::normalize(bottomLeft - topRight)));
    insertVec3(m_vertexData, topLeftNormal);
    insertVec2(m_vertexData, topLeftTexture);

    insertVec3(m_vertexData, bottomLeft);
    glm::vec3 bottomLeftNormal = glm::normalize(glm::cross(glm::normalize(bottomLeft- topRight), glm::normalize(topRight - topLeft)));
    insertVec3(m_vertexData, bottomLeftNormal);
    insertVec2(m_vertexData, bottomLeftTexture);

    insertVec3(m_vertexData, topRight);
    glm::vec3 topRightNormal = glm::normalize(glm::cross(glm::normalize(topRight - topLeft), glm::normalize(topLeft - bottomLeft)));
    insertVec3(m_vertexData, topRightNormal);
    insertVec2(m_vertexData, topRightTexture);

    // Second triangle
    insertVec3(m_vertexData, topRight);
    glm::vec3 topRightNormal2 = glm::normalize(glm::cross(glm::normalize(topRight - bottomLeft), glm::normalize(bottomLeft - bottomRight)));
    insertVec3(m_vertexData, topRightNormal2);
    insertVec2(m_vertexData, topRightTexture);

    insertVec3(m_vertexData, bottomLeft);
    glm::vec3 bottomLeftNormal2 = glm::normalize(glm::cross(glm::normalize(bottomLeft- bottomRight), glm::normalize(bottomRight - topRight)));
    insertVec3(m_vertexData, bottomLeftNormal2);
    insertVec2(m_vertexData, bottomLeftTexture);

    insertVec3(m_vertexData, bottomRight);
    glm::vec3 bottomRightNormal = glm::normalize(glm::cross(glm::normalize(bottomRight - topRight), glm::normalize(topRight - bottomLeft)));
    insertVec3(m_vertexData, bottomRightNormal);
    insertVec2(m_vertexData, bottomRightTexture);
}

void Cone::makeTileSide(glm::vec3 topLeft,
                        glm::vec3 topRight,
                        glm::vec3 bottomLeft,
                        glm::vec3 bottomRight,
                        glm::vec3 bottomMiddle,
                        glm::vec2 topLeftTexture,
                        glm::vec2 topRightTexture,
                        glm::vec2 bottomLeftTexture,
                        glm::vec2 bottomRightTexture) {
    // Calculate the slope of the cone
    float radius = m_radius;
    float height = 1.0;
    float slope = height / glm::sqrt(radius * radius + height * height);

    // Calculate the normal for the bottom vertices
    auto calculateNormal = [slope](const glm::vec3& position) -> glm::vec3 {
        glm::vec3 radialComponent = glm::normalize(glm::vec3(position.x, 0, position.z));
        return glm::normalize(glm::vec3(radialComponent.x, 0.5, radialComponent.z));
    };

    // First triangle
    // Insert the topLeft vertex and its normal
    insertVec3(m_vertexData, topLeft);
    glm::vec3 topLeftNormal;
    if (topLeft.y == 0.5) {
        topLeftNormal = calculateNormal(bottomMiddle);
    }
    else {
        topLeftNormal = calculateNormal(topLeft);
    }
    insertVec3(m_vertexData, topLeftNormal);
    insertVec2(m_vertexData, topLeftTexture);

    // Insert the bottomLeft vertex and its normal
    insertVec3(m_vertexData, bottomLeft);
    glm::vec3 bottomLeftNormal = calculateNormal(bottomLeft);
    insertVec3(m_vertexData, bottomLeftNormal);
    insertVec2(m_vertexData, bottomLeftTexture);

    // Insert the topRight vertex and its normal
    insertVec3(m_vertexData, topRight);
    glm::vec3 topRightNormal;
    if (topLeft.y == 0.5) {
        topRightNormal = topLeftNormal;
    }
    else {
        topRightNormal = calculateNormal(topRight);
    }
    insertVec3(m_vertexData, topRightNormal);
    insertVec2(m_vertexData, topRightTexture);

    // Second triangle
    insertVec3(m_vertexData, topRight);
    insertVec3(m_vertexData, topRightNormal); // Reuse the topLeftNormal
    insertVec2(m_vertexData, topRightTexture);

    insertVec3(m_vertexData, bottomLeft);
    insertVec3(m_vertexData, bottomLeftNormal); // Reuse the bottomRightNormal
    insertVec2(m_vertexData, bottomLeftTexture);

    // Insert the bottomRight vertex and its normal
    insertVec3(m_vertexData, bottomRight);
    glm::vec3 bottomRightNormal = calculateNormal(bottomRight);
    insertVec3(m_vertexData, bottomRightNormal);
    insertVec2(m_vertexData, bottomRightTexture);
}

void Cone::makeSide(float currentTheta, float nextTheta, float thetaStep) {
    float yStep = 1.0 / m_param1;
    float step = m_radius / m_param1;

    for (int i = 0; i < m_param1; i++) {
        glm::vec3 topLeft = {i * step * glm::sin(currentTheta), 0.5 - i * yStep, i * step * glm::cos(currentTheta)};
        glm::vec3 topRight = {i * step * glm::sin(nextTheta), 0.5 - i * yStep, i * step * glm::cos(nextTheta)};
        glm::vec3 bottomLeft = {(i + 1) * step * glm::sin(currentTheta), 0.5 - (i + 1) * yStep, (i + 1) * step * glm::cos(currentTheta)};
        glm::vec3 bottomRight = {(i + 1) * step * glm::sin(nextTheta), 0.5 - (i + 1) * yStep, (i + 1) * step * glm::cos(nextTheta)};

        glm::vec3 bottomMiddle = {(i + 1) * step * glm::sin(currentTheta + thetaStep / 2), 0.5 - (i + 1) * yStep, (i + 1) * step * glm::cos(currentTheta + thetaStep / 2)};

        std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeft, topRight, bottomLeft, bottomRight);
        glm::vec2 topLeftTexture = tileTextureUV.at(0);
        glm::vec2 topRightTexture = tileTextureUV.at(1);
        glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
        glm::vec2 bottomRightTexture = tileTextureUV.at(3);

        makeTileSide(topLeft, topRight, bottomLeft, bottomRight, bottomMiddle, topLeftTexture, topRightTexture, bottomLeftTexture, bottomRightTexture);
    }
}

void Cone::makeCap(float currentTheta, float nextTheta) {
    float step = m_radius / m_param1;

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

std::vector<glm::vec2> Cone::calculateTileTextureUV(glm::vec3 topLeft,
                                                    glm::vec3 topRight,
                                                    glm::vec3 bottomLeft,
                                                    glm::vec3 bottomRight) {
    std::vector<glm::vec2> tileTextureUV;

    glm::vec2 topLeftTexture;
    if (isTexture) {
        topLeftTexture = coneTexture(topLeft, repeatU, repeatV);
    }
    else {
        topLeftTexture = {0,0};
    }
    tileTextureUV.push_back(topLeftTexture);

    glm::vec2 topRightTexture;
    if (isTexture) {
        topRightTexture = coneTexture(topRight, repeatU, repeatV);
    }
    else {
        topRightTexture = {0,0};
    }
    tileTextureUV.push_back(topRightTexture);

    glm::vec2 bottomLeftTexture;
    if (isTexture) {
        bottomLeftTexture = coneTexture(bottomLeft, repeatU, repeatV);
    }
    else {
        bottomLeftTexture = {0,0};
    }
    tileTextureUV.push_back(bottomLeftTexture);

    glm::vec2 bottomRightTexture;
    if (isTexture) {
        bottomRightTexture = coneTexture(bottomRight, repeatU, repeatV);
    }
    else {
        bottomRightTexture = {0,0};
    }
    tileTextureUV.push_back(bottomRightTexture);

    return tileTextureUV;
}

glm::vec2 Cone::coneTexture(glm::vec3 point, float repeatU, float repeatV) {
    float x = point.x;
    float y = point.y;
    float z = point.z;

    float epsilon = 1e-3;  // Added epsilon for floating-point precision issues

    float u = 0;
    float v = 0;
    if (std::abs(y + 0.5) < epsilon) {
        u = x + 0.5;
        v = z + 0.5;
    }
    else {
        // For U (azimuthal angle):
        float theta = atan2(z, x);
        if (theta < 0) {
            u = -theta / (2 * M_PI);
        } else {
            u = 1 - theta / (2 * M_PI);
        }

        // For V (height along the cone):
        v = y + 0.5;
    }

    int segment_u = static_cast<int>(u * repeatU);
    float u_prime = u * repeatU - segment_u;

    int segment_v = static_cast<int>(v * repeatV);
    float v_prime = v * repeatV - segment_v;

    glm::clamp(u_prime, 0.0f, 1.0f);
    glm::clamp(v_prime, 0.0f, 1.0f);
    return glm::vec2(u_prime, v_prime);
}

void Cone::setVertexData() {
    float thetaStep = glm::radians(360.f / m_param2);
    float currentTheta;
    float nextTheta;
    for (int i = 0; i < m_param2; i++) {
        currentTheta = i * thetaStep;
        nextTheta = (i + 1) * thetaStep;
        makeSide(currentTheta, nextTheta, thetaStep);
        makeCap(currentTheta, nextTheta);
    }
}
