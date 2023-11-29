#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <cstdint>
#include <QObject>
#include <QImage>
#include <shapes/common.h>

class Cone
{
public:
    void updateParams(int param1, int param2, bool isTexture, float repeatU, float repeatV, QString imgPath);
    std::vector<float> generateShape() { return m_vertexData; }

private:
    void setVertexData();

    void makeTile(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight,
                  glm::vec2 topLeftTexture,
                  glm::vec2 topRightTexture,
                  glm::vec2 bottomLeftTexture,
                  glm::vec2 bottomRightTexture);
    void makeTileSide(glm::vec3 topLeft,
                      glm::vec3 topRight,
                      glm::vec3 bottomLeft,
                      glm::vec3 bottomRight,
                      glm::vec3 bottomMiddle,
                      glm::vec2 topLeftTexture,
                      glm::vec2 topRightTexture,
                      glm::vec2 bottomLeftTexture,
                      glm::vec2 bottomRightTexture);
    void makeSide(float currentTheta, float nextTheta, float thetaStep);
    void makeCap(float currentTheta, float nextTheta);
    void makeCylinder();

    std::vector<glm::vec2> calculateTileTextureUV(glm::vec3 topLeft,
                                                  glm::vec3 topRight,
                                                  glm::vec3 bottomLeft,
                                                  glm::vec3 bottomRight);
    glm::vec2 coneTexture(glm::vec3 point, float repeatU, float repeatV);

    std::vector<float> m_vertexData;
    int m_param1;
    int m_param2;
    float m_radius = 0.5;

    std::map<QString, ImageData> imageCache;
    bool isTexture;
    float repeatU;
    float repeatV;
    QString imgPath;
};

