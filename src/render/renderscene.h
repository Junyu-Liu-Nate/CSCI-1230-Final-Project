#pragma once

#include "utils/scenedata.h"
#include "utils/sceneparser.h"
#include "camera.h"

// A class representing a scene to be ray-traced

// Feel free to make your own design choices for RayTraceScene, the functions below are all optional / for your convenience.
// You can either implement and use these getters, or make your own design.
// If you decide to make your own design, feel free to delete these as TAs won't rely on them to grade your assignments.

class RenderScene
{
public:
    int sceneWidth;
    int sceneHeight;
    RenderData sceneMetaData;
    Camera sceneCamera;

    RenderScene() : sceneWidth(0), sceneHeight(0), sceneMetaData(), sceneCamera() {};
    RenderScene(int width, int height, float nearPlane, float farPlane, const RenderData &metaData);
    void updateCamera(float nearPlane, float farPlane, const RenderData &metaData);

    // The getter of the width of the scene
    const int& width() const;

    // The getter of the height of the scene
    const int& height() const;

    // The getter of the global data of the scene
    const SceneGlobalData& getGlobalData() const;

    // The getter of the shared pointer to the camera instance of the scene
    const Camera& getCamera() const;
};
