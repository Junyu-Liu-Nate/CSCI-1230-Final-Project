#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <cstdint>
#include <QObject>
#include <QImage>
#include <shapes/common.h>

class Square
{
public:
    void updateParams(bool isTexture, float repeatU, float repeatV, QString imgPath);
    std::vector<float> generateShape() { return m_vertexData; }

private:
    void setVertexData();
    void makeface(bool flag);
    void insertData(glm::vec3 vertexData,glm::vec3 normal,glm::vec2 texture);
    std::vector<glm::vec2> calculateTileTextureUV(glm::vec3 topLeft,
                                                  glm::vec3 topRight,
                                                  glm::vec3 bottomLeft,
                                                  glm::vec3 bottomRight);
    glm::vec2 cubeTexture(glm::vec3 point, float repeatU, float repeatV);

    std::vector<float> m_vertexData;
    std::map<QString, ImageData> imageCache;
    bool isTexture;
    float repeatU;
    float repeatV;
    QString imgPath;
};
