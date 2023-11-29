#include <stdexcept>
#include "camera.h"
#include <iostream>

void Camera::setup(int width, int height, float nearPlane, float farPlane, glm::vec4 pos, glm::vec4 look, glm::vec4 up, float heightAngle, float aperture, float focalLength) {
    cameraPos = pos;
    cameraPos.w = 1;
    cameraLook = look;
    cameraUp = up;

    cameraNearPlane = nearPlane;
    cameraFarPlane = farPlane;

    calculateViewMatrix();

    aspectRatio = (width * 1.0) / height;

    cameraHeightAngle = heightAngle;
    cameraAperture = aperture;
    cameraFocalLength = focalLength;

    calculateProjectionMatrix();
}

void Camera::calculateViewMatrix() {
    // Note: convert the vec4 to vec3 for cross product operation
    glm::vec3 w = -glm::vec3(cameraLook);
    w = glm::normalize(w);
    glm::vec3 v = glm::vec3(cameraUp) - glm::dot(glm::vec3(cameraUp), w) * w;
    v = glm::normalize(v);
    glm::vec3 u = glm::cross(v, w);

    // Rotation matrix
    glm::vec4 wExpand = glm::vec4(w, 0);
    glm::vec4 vExpand = glm::vec4(v, 0);
    glm::vec4 uExpand = glm::vec4(u, 0);

    glm::mat4 matRotateT = glm::mat4(uExpand, vExpand, wExpand, glm::vec4(0, 0, 0, 1));
    glm::mat4 matRotate = glm::transpose(matRotateT);

    // Translation matrix
    glm::mat4 matTranslate = glm::mat4(glm::vec4(1, 0, 0, 0), glm::vec4(0, 1, 0, 0), glm::vec4(0, 0, 1, 0), glm::vec4(-glm::vec3(cameraPos), 1));

    viewMatrix = matRotate * matTranslate;
}

void Camera::calculateProjectionMatrix() {
//    float cameraWidthAngle = cameraHeightAngle * aspectRatio;
    float cameraWidthAngle = 2.0f * glm::atan(glm::tan(cameraHeightAngle / 2.0f) * aspectRatio);
    float scaleX = 1 / (cameraFarPlane * glm::tan(cameraWidthAngle / 2));
    float scaleY = 1 / (cameraFarPlane * glm::tan(cameraHeightAngle / 2));
    float scaleZ = 1 / cameraFarPlane;
    glm::mat4 scaleMatrix = glm::mat4(glm::vec4(scaleX, 0, 0, 0),
                                      glm::vec4(0, scaleY, 0, 0),
                                      glm::vec4(0, 0, scaleZ, 0),
                                      glm::vec4(0, 0, 0, 1));

    float c = - cameraNearPlane / cameraFarPlane;
    glm::mat4 parallelMatrix = glm::mat4(glm::vec4(1, 0, 0, 0),
                                         glm::vec4(0, 1, 0, 0),
                                         glm::vec4(0, 0, 1/(1+c), -1),
                                         glm::vec4(0, 0, -c/(1+c), 0));

    glm::mat4 remapMatrix = glm::mat4(glm::vec4(1, 0, 0, 0),
                                      glm::vec4(0, 1, 0, 0),
                                      glm::vec4(0, 0, -2, 0),
                                      glm::vec4(0, 0, -1, 1));

    projectionMatrix = remapMatrix * parallelMatrix * scaleMatrix;
}

glm::mat4 Camera::getViewMatrix() const {
    return viewMatrix;
}

glm::mat4 Camera::getViewMatrixInverse() const {
    return glm::inverse(viewMatrix);
}

glm::mat4 Camera::getProjectMatrix() const {
    return projectionMatrix;
}

float Camera::getAspectRatio() const {
    return aspectRatio;
}

float Camera::getHeightAngle() const {
    return cameraHeightAngle;
}

float Camera::getFocalLength() const {
    return cameraFocalLength;
}

float Camera::getAperture() const {
    return cameraAperture;
}
