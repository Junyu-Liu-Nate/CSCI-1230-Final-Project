#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>

#include "render/renderscene.h"
#include "shapes/sphere.h"
#include "shapes/cube.h"
#include "shapes/cone.h"
#include "shapes/cylinder.h"
#include "shapes/mesh.h"
#include "shapes/terrain.h"
#include "settings.h"
#include "shapes/particle.h"
#include "shapes/square.h"
class Realtime : public QOpenGLWidget
{
public:
    Realtime(QWidget *parent = nullptr);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:
    void initializeGL() override;                       // Called once at the start of the program
    void paintGL() override;                            // Called whenever the OpenGL context changes or by an update() request
    void resizeGL(int width, int height) override;      // Called when window size changes

private:
    // ======= GL-related
    void setupShapesGL();
    void setupTerrainGL();

    // ======= Shapes-related
    GLuint m_shader; // Stores id of main shader program - default.vert/.frag
    GLuint m_vbo; // Stores id of vbo
    GLuint m_vao; // Stores id of vao

    QImage m_image; // Texture image
    GLuint m_texture; // Stores id of geometry texture mapping

    RenderData metaData; // Parsed scene data by scene paser
    RenderScene renderScene;

    std::vector<std::vector<float>> shapeDataList;
    std::vector<glm::mat4> modelMatrixList;
    glm::mat4 m_view  = glm::mat4(1);
    glm::mat4 m_proj  = glm::mat4(1);

    std::vector<float> vboData; // Flat vector to store all the vertices and normals for the VBO.
    std::vector<int> shapeStartIndices; // Vector to store the starting index of each shape in the VBO.
    std::vector<int> shapeSizes; // Vector to store the number of elements (vertices+normals) of each shape.

    void generateScreen();
    void setupShapeData();
    void setupLightData();
    std::vector<float> calculateDistanceFactors();
    void resetScene();
    void paintGeometry();

    int shapeParameter1Saved = settings.bumpiness;
    int shapeParameter2Saved = settings.shapeParameter2;
    QString texture_filepath_saved = QString::fromStdString("");


    // ====== Particle-related
    void paintParticle();
    void setupParticleGL();
    void setupParticle();
    void update_particle_vbo();

    std::shared_ptr<ParticleSystem> particles = std::make_shared<ParticleSystem>();

    GLuint m_particle_shader; // Stores id of particle shader program
    GLuint m_particle_texture;
    GLuint m_particle_vbo;// Stores id of particle vbo
    GLuint m_particle_vao;// Stores id of particle vao
    std::vector<float> m_particle_data;
    QImage m_particle_image; // Texture image for terrain

    int staticParticleNum = 0;
    std::vector<std::vector<float>> staticShapeDataList;
    std::vector<glm::mat4> staticMatrixList;


    // ====== Terrain-related
    GLuint m_terrain_shader; // Stores id of terrain shader program - terrain.vert/.frag
    GLuint m_terrain_vbo; // Stores id of terrain vbo
    GLuint m_terrain_vao; // Stores id of terrain vao

    std::vector<float> terrainData;
    glm::mat4 terrainModelMatrix;

    std::vector<float> terrainVboData;
    int terrainStartIndex;
    int terrainSize;

    QImage m_terrain_image; // Texture image for terrain
    GLuint m_terrain_texture; // Stores id of geometry texture mapping

    TerrainGenerator terrainGenerator;
    GLuint m_collision_texture; // Store id of collision map
    std::vector<GLuint> matrixData;

    void setupTerrainData();

    void updateTerrainCollisionMap();
    void paintTerrain();

    float accumulateRate = 0.001;

    // ======= Weather-related
    int timeTracker = 0;
    int dayTimeSaved = settings.time;
    bool isSunMove = settings.sun;
    int snowTimer = 0;
    int sunTimer = 0;

    float rotationSpeedScale = 0.25;
    glm::vec3 sunlightColor = {0.0f,0.0f,0.0f};
    glm::vec4 sunlightOriginalDirection = {-1,0,0,0};
    glm::vec3 sunlightDirection = {-1,0.0f,0.0f};

    void setSunlightDirectionAccordingToTime();
    void updateSunlight(glm::vec4 originalDirection);

    // ======= Frame-related
    GLuint m_defaultFBO;
    int m_fbo_width;
    int m_fbo_height;
    int m_screen_width;
    int m_screen_height;

    GLuint m_frame_shader;
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;
    GLuint m_fbo;
    GLuint m_fbo_texture;
    GLuint m_fbo_renderbuffer;

    void makeFBO();
    void paintFrame(GLuint texture);

    // ======= Others
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
    int m_devicePixelRatio;
};
