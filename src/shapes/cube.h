#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <cstdint>
#include <QObject>
#include <QImage>
#include <shapes/common.h>

class Cube
{
public:
    void updateParams(int param1, bool isTexture, float repeatU, float repeatV, QString imgPath);
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
    void makeFace(glm::vec3 topLeft,
                  glm::vec3 topRight,
                  glm::vec3 bottomLeft,
                  glm::vec3 bottomRight,
                  int faceIndex);

    std::vector<glm::vec2> calculateTileTextureUV(glm::vec3 topLeft,
                                                  glm::vec3 topRight,
                                                  glm::vec3 bottomLeft,
                                                  glm::vec3 bottomRight);
    glm::vec2 cubeTexture(glm::vec3 point, float repeatU, float repeatV);

    std::vector<float> m_vertexData;
    int m_param1;

    std::map<QString, ImageData> imageCache;
    bool isTexture;
    float repeatU;
    float repeatV;
    QString imgPath;
};
