#include "square.h"



void Square::updateParams(bool isTexture, float repeatU, float repeatV, QString imgPath) {
    m_vertexData = std::vector<float>();

    this->isTexture = isTexture;
    this->repeatU = repeatU;
    this->repeatV = repeatV;
    this->imgPath = imgPath;
    setVertexData();
}
void Square::insertData(glm::vec3 vertexData,glm::vec3 normal,glm::vec2 texture){
    insertVec3(m_vertexData,(vertexData));
    insertVec3(m_vertexData,(normal));
    if(isTexture){insertVec2(m_vertexData,(texture));}
}
void Square::makeface(bool flag) {
    int m=4;
    glm::vec3 normal = flag ? glm::vec3(0, 0, 1) : glm::vec3(0, 0, -1);

    float step = 1.0f / m; // 步长，根据m计算

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < m; ++j) {
            // 计算当前小正方形的四个顶点
            glm::vec3 topLeft((-0.5 + i * step), (0.5 - j * step), flag ? 0.01 : -0.01);
            glm::vec3 bottomLeft((-0.5 + i * step), (0.5 - (j + 1) * step), flag ? 0.01 : -0.01);
            glm::vec3 bottomRight((-0.5 + (i + 1) * step), (0.5 - (j + 1) * step), flag ? 0.01 : -0.01);
            glm::vec3 topRight((-0.5 + (i + 1) * step), (0.5 - j * step), flag ? 0.01 : -0.01);

            // 计算纹理坐标
            std::vector<glm::vec2> tileTextureUV = calculateTileTextureUV(topLeft, topRight, bottomLeft, bottomRight);
            glm::vec2 topLeftTexture = tileTextureUV.at(0);
            glm::vec2 topRightTexture = tileTextureUV.at(1);
            glm::vec2 bottomLeftTexture = tileTextureUV.at(2);
            glm::vec2 bottomRightTexture = tileTextureUV.at(3);

            // 插入两个三角形的顶点、法线和纹理坐标
            insertData(topLeft, normal, topLeftTexture);
            insertData(bottomLeft, normal, bottomLeftTexture);
            insertData(bottomRight, normal, bottomRightTexture);

            insertData(topRight, normal, topRightTexture);
            insertData(topLeft, normal, topLeftTexture);
            insertData(bottomRight, normal, bottomRightTexture);
        }
    }
}
void Square::setVertexData(){

// front face
    makeface(true);
// back face
    makeface(false);
}


std::vector<glm::vec2> Square::calculateTileTextureUV(glm::vec3 topLeft,
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

glm::vec2 Square::cubeTexture(glm::vec3 point, float repeatU, float repeatV) {
    float x = point.x;
    float y = point.y;
    float z = point.z;
     float u, v;
    const float epsilon = 1e-5f;  // Epsilon for floating point comparisons
    if (std::abs(z -0.01) < epsilon) {
        u = (x + 0.5);
        v = (y + 0.5);
    }
    else {  // z + 0.01
        u = (-x + 0.5);
        v = (y + 0.5);
    }


    int segment_u = static_cast<int>(u * repeatU);
    float u_prime = u * repeatU - segment_u;

    int segment_v = static_cast<int>(v * repeatV);
    float v_prime = v * repeatV - segment_v;

    return glm::vec2(u_prime, v_prime);
}

