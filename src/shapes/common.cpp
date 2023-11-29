#include "common.h"

std::vector<RGBA> loadImage(const QString &filePath, int &width, int &height) {
    QImage myImage;
    if (!myImage.load(filePath)) {
        std::cerr << "Failed to load in image" << std::endl;
        return {};
    }

    myImage = myImage.convertToFormat(QImage::Format_RGBX8888);
    QByteArray arr = QByteArray::fromRawData((const char*) myImage.bits(), myImage.sizeInBytes());

    width = myImage.width();
    height = myImage.height();

    std::vector<RGBA> image;
    image.reserve(width * height);
    for (int i = 0; i < arr.size() / 4; i++){
        image.push_back(RGBA{
            (std::uint8_t) arr[4*i],
            (std::uint8_t) arr[4*i+1],
            (std::uint8_t) arr[4*i+2],
            (std::uint8_t) arr[4*i+3]
        });
    }

    return image;
}

void insertVec3(std::vector<float>& data, const glm::vec3& v) {
    data.push_back(v.x);
    data.push_back(v.y);
    data.push_back(v.z);
}

void insertVec2(std::vector<float>& data, const glm::vec2& v) {
    data.push_back(v.x);
    data.push_back(v.y);
}
