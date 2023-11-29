#pragma once

#include <glm/glm.hpp>

// A class representing a virtual camera.

class Camera {
public:
    glm::vec4 cameraPos;
    glm::vec4 cameraLook;
    glm::vec4 cameraUp;

    float cameraNearPlane;
    float cameraFarPlane;

    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    float aspectRatio;

    float cameraHeightAngle; // The height angle of the camera in RADIANS
    float cameraAperture;    // Only applicable for depth of field
    float cameraFocalLength; // Only applicable for depth of field

    // Setup all camera parameters
    void setup(int width, int height, float nearPlane, float farPlane, glm::vec4 pos, glm::vec4 look, glm::vec4 up, float heightAngle, float aperture, float focalLength);

    void calculateViewMatrix();
    void calculateProjectionMatrix();

    // Returns the view matrix for the current camera settings.
    // You might also want to define another function that return the inverse of the view matrix.
    glm::mat4 getViewMatrix() const;
    glm::mat4 getViewMatrixInverse() const;
    glm::mat4 getProjectMatrix() const;

    // Returns the aspect ratio of the camera.
    float getAspectRatio() const;

    // Returns the height angle of the camera in RADIANS.
    float getHeightAngle() const;

    // Returns the focal length of this camera.
    float getFocalLength() const;

    // Returns the focal length of this camera.
    float getAperture() const;
};
