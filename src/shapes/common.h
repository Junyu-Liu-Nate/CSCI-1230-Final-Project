#pragma once

#include <cstdint>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <glm/glm.hpp>
#include <cstdint>
#include <QObject>
#include <QImage>

struct RGBA {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a = 255;
};

struct ImageData {
    std::vector<RGBA> data;
    int width;
    int height;
};

std::vector<RGBA> loadImage(const QString &filePath, int &width, int &height);
void insertVec3(std::vector<float>& data, const glm::vec3& v);
void insertVec2(std::vector<float>& data, const glm::vec2& v);
