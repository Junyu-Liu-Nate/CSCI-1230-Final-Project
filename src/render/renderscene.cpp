#include <stdexcept>
#include "renderscene.h"
#include "utils/sceneparser.h"

RenderScene::RenderScene(int width, int height, float nearPlane, float farPlane, const RenderData &metaData) {
    sceneWidth = width;
    sceneHeight = height;
    sceneMetaData = metaData;

    // Setup the camera
    SceneCameraData cameraData = sceneMetaData.cameraData;
    sceneCamera.setup(sceneWidth, sceneHeight, nearPlane, farPlane, cameraData.pos, cameraData.look, cameraData.up, cameraData.heightAngle, cameraData.aperture, cameraData.focalLength);
}

void RenderScene::updateCamera(float nearPlane, float farPlane, const RenderData &metaData) {
    SceneCameraData cameraData = metaData.cameraData;
    sceneCamera.setup(sceneWidth, sceneHeight, nearPlane, farPlane, cameraData.pos, cameraData.look, cameraData.up, cameraData.heightAngle, cameraData.aperture, cameraData.focalLength);
}

const int& RenderScene::width() const {
    return sceneWidth;
}

const int& RenderScene::height() const {
    return sceneHeight;
}

const SceneGlobalData& RenderScene::getGlobalData() const {
    return sceneMetaData.globalData;
}

const Camera& RenderScene::getCamera() const {
    return sceneCamera;
}
