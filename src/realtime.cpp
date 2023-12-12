#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include <QDir>
#include <QDebug>
#include "settings.h"
#include <QtConcurrent>
#include "utils/shaderloader.h"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "settings.h"
#include "utils/sceneparser.h"
//#include <omp.h>

struct ShapeAndModel {
    std::vector<float> shapeData;
    glm::mat4 modelMatrix;
};

Realtime::Realtime(QWidget *parent)
    : QOpenGLWidget(parent)
{
    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Delete framebuffers
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    // Delete terrain-related resources
    glDeleteTextures(1, &m_terrain_texture);
    glDeleteBuffers(1, &m_terrain_vbo);
    glDeleteVertexArrays(1, &m_terrain_vao);

    // Delete particle-related resources
    glDeleteTextures(1, &m_particle_texture);
    glDeleteBuffers(1, &m_particle_vbo);
    glDeleteVertexArrays(1, &m_particle_vao);

    // Delete other textures and buffers if any
    glDeleteTextures(1, &m_texture);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);

    // Delete shaders
    glDeleteProgram(m_shader);
    glDeleteProgram(m_particle_shader);
    glDeleteProgram(m_terrain_shader);
    glDeleteProgram(m_frame_shader);

    // Delete collision texture
    glDeleteTextures(1, &m_collision_texture);

    this->doneCurrent();
}


// ********************************* GL *********************************
void Realtime::initializeGL() {
    m_devicePixelRatio = this->devicePixelRatio();

    // Texture and FBO related
    m_defaultFBO = 2;
    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    m_timer = startTimer(1000/30);
    m_elapsedTimer.start();

    // ====== Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // ====== Load the main(default) shader
    m_shader = ShaderLoader::createShaderProgram("resources/shaders/default.vert", "resources/shaders/default.frag");
    // Generate VBO
    glGenBuffers(1, &m_vbo);
    // Generate VAO
    glGenVertexArrays(1, &m_vao);

    // Generate texture image
    glGenTextures(1, &m_texture);
    glActiveTexture(GL_TEXTURE3); // Use texture slot 3!!!
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    QString currentDir = QDir::currentPath();

    QString texture_filepath = currentDir + QString::fromStdString("/scenefiles/textures/snowflake.png");
    m_image = QImage(texture_filepath);
    if (m_image.isNull()) {
        // Handle error: Image didn't load
        std::cerr << "Failed to load texture image: " << texture_filepath.toStdString() << std::endl;
        std::cerr << "Continue with no texture image." << std::endl;
        glUniform1f(glGetUniformLocation(m_shader, "isTexture"), -1.0);
    }
    // Format image to fit OpenGL
    m_image = m_image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    texture_filepath_saved = texture_filepath;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.width(), m_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());

    // ====== Generate Terrain-related stuff
    //Generate Particle-related stuff
    m_particle_shader=ShaderLoader::createShaderProgram("resources/shaders/particle.vert", "resources/shaders/particle.frag");
    glGenBuffers(1, &m_particle_vbo);
    glGenVertexArrays(1, &m_particle_vao);
    glGenTextures(1,&m_particle_texture);

    // Generate Terrain-related stuff
    m_terrain_shader = ShaderLoader::createShaderProgram("resources/shaders/terrain.vert", "resources/shaders/terrain.frag");
    // Generate terrain VBO
    glGenBuffers(1, &m_terrain_vbo);
    // Generate terrain VAO
    glGenVertexArrays(1, &m_terrain_vao);
    // Generate texture image
    glGenTextures(1, &m_terrain_texture);
    matrixData = std::vector<GLuint>(100 * 100, 0);

    glGenTextures(1, &m_terrain_texture); // Use texture slot 1!!!
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_terrain_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    currentDir = QDir::currentPath();
    texture_filepath = currentDir + QString::fromStdString("/scenefiles/textures/mountain_3.png");
    m_terrain_image = QImage(texture_filepath);
    if (m_terrain_image.isNull()) {
        // Handle error: Image didn't load
        std::cerr << "Failed to load texture image: " << texture_filepath.toStdString() << std::endl;
        std::cerr << "Continue with no texture image." << std::endl;
        glUniform1f(glGetUniformLocation(m_terrain_shader, "isTexture"), -1.0);
    }
    // Format image to fit OpenGL
    m_terrain_image = m_terrain_image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    texture_filepath_saved = texture_filepath;
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, m_terrain_texture);
    // Load image into texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_terrain_image.width(), m_terrain_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_terrain_image.bits());

    glGenTextures(1, &m_collision_texture);
    glActiveTexture(GL_TEXTURE2); // Use texture slot 2!!!
    glBindTexture(GL_TEXTURE_2D, m_collision_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // ====== Texture shader - operates on FBO
    m_frame_shader = ShaderLoader::createShaderProgram("resources/shaders/frame.vert", "resources/shaders/frame.frag");
    // Generate screen mesh
    generateScreen();
    makeFBO();
}

void Realtime::generateScreen() {
    std::vector<GLfloat> fullscreen_quad_data = {
        // Positions    // UV Coordinates
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f, // Top Left
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom Left
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f,  // Bottom Right
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f,  // Top Right
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f, // Top Left
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f   // Bottom Right
    };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (const void*)(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Realtime::makeFBO(){
    // Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, &m_fbo_texture); // Generate fbo texture
    glActiveTexture(GL_TEXTURE0); // Set the active texture slot to texture slot 0
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture); // Bind fbo texture

    // Load empty image into fbo texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set min and mag filters' interpolation mode to linear
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind fbo texture

    // Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
}

void Realtime::paintGL() {
    // Anything requiring OpenGL calls every frame should be done here
    // Update timers
    timeTracker += 1;
    if (settings.snow) {snowTimer += 1;}
    if (settings.sun) {sunTimer += 1;}

    if (settings.heightMapPath != heightMapPathSaved) {
        staticParticleNum = 0;
        staticShapeDataList.clear();
        staticMatrixList.clear();

        m_view = glm::mat4(1.0f);
        m_proj = glm::mat4(1.0f);

        if (!settings.sceneFilePath.empty()) {
            // Create scene
            renderScene = RenderScene(size().width() * m_devicePixelRatio,
                                      size().height() * m_devicePixelRatio,
                                      settings.nearPlane,
                                      settings.farPlane,
                                      metaData);
            setupTerrainGL();
            // Setup camera data from the scene
            m_view = renderScene.sceneCamera.getViewMatrix();
            m_proj = renderScene.sceneCamera.getProjectMatrix();
        }

        heightMapPathSaved = settings.heightMapPath;
    }

    // Bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Call glViewport
    glViewport(0, 0, m_screen_width, m_screen_height);
    // Clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!settings.sceneFilePath.empty()) {
        // ====== Draw with terrain shader
        if (settings.sun) {
            updateSunlight(sunlightOriginalDirection);
        }
        paintTerrain();

        // ====== Draw with default shader
        if (settings.snow) {
            paintGeometry();
        }
    }

    // ====== Draw with frame shader
    // Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width, m_screen_height);
    // Call paintFrame to draw our FBO color attachment texture
    paintFrame(m_fbo_texture);
}

void Realtime::paintFrame(GLuint texture) {
    glUseProgram(m_frame_shader);

    // Define gradient colors and direction
    glm::vec2 gradientDirection = glm::vec2(0.0f, 1.0f); // Direction from top to bottom

    // Normalize the light direction
    glm::vec3 normalizedLightDirection = glm::normalize(sunlightDirection);
    // Dot product with the y-axis
    float dotProduct = glm::dot(normalizedLightDirection, glm::vec3(0.0f, 1.0f, 0.0f));
    // Use the absolute value of the dot product to ensure we don't get negative values
    float blendFactor = glm::abs(dotProduct);

    // Use blendFactor from updateSunlight for the gradient transition
    float nightTransitionFactor = 1.0f - blendFactor * 0.6; // Invert to darken during night

    // Define base gradient colors
    glm::vec3 baseWarmGradientColor = glm::vec3(0.25f, 0.1f, 0.05f); // Slightly warm color for the gradient start
    glm::vec3 baseCoolGradientColor = glm::vec3(0.05f, 0.1f, 0.5f); // Cool color for the gradient end

    // Adjust the gradient colors based on the nightTransitionFactor
    glm::vec3 gradientStartColor;
    glm::vec3 gradientEndColor;
    if (normalizedLightDirection.y <= 0) {
        gradientStartColor = glm::mix(baseWarmGradientColor, sunlightColor, 0.3f);
        gradientEndColor = glm::mix(baseCoolGradientColor, sunlightColor, 0.3f);
    }
    else {
        gradientStartColor = glm::mix(baseWarmGradientColor, sunlightColor, 0.3f) * nightTransitionFactor;
        gradientEndColor = glm::mix(baseCoolGradientColor, sunlightColor, 0.3f) * nightTransitionFactor;
    }

    // Set the gradient uniforms
    glUniform3f(glGetUniformLocation(m_frame_shader, "gradientStartColor"), gradientStartColor.r, gradientStartColor.g, gradientStartColor.b);
    glUniform3f(glGetUniformLocation(m_frame_shader, "gradientEndColor"), gradientEndColor.r, gradientEndColor.g, gradientEndColor.b);
    glUniform2f(glGetUniformLocation(m_frame_shader, "gradientDirection"), gradientDirection.x, gradientDirection.y);

    // Set bool uniform on whether or not to filter the texture drawn
    glUniform1i(glGetUniformLocation(m_frame_shader, "isPerPixelFilter"), settings.perPixelFilter);
    glUniform1i(glGetUniformLocation(m_frame_shader, "isKernelFilter"), settings.kernelBasedFilter);

    glUniform1i(glGetUniformLocation(m_frame_shader, "isFXAA"), settings.extraCredit3);

    glUniform1f(glGetUniformLocation(m_frame_shader, "screenWidth"), m_screen_width);
    glUniform1f(glGetUniformLocation(m_frame_shader, "screenHeight"), m_screen_height);

    glBindVertexArray(m_fullscreen_vao);

    // Bind "texture" to slot 0
    glActiveTexture(GL_TEXTURE0);           // Activate texture slot 0
    glBindTexture(GL_TEXTURE_2D, texture);  // Bind the texture to the active texture slot

    // Note that the textureImg uniform is implicitly set to use texture slot 0
    // because GL_TEXTURE0 is active when the texture is bound.

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Realtime::paintGeometry() {
    //    paintParticle();
    // Bind Vertex Data
    glBindVertexArray(m_vao);

    // Activate the shader program by calling glUseProgram with `m_shader`
    glUseProgram(m_shader);

    glUniform1i(glGetUniformLocation(m_shader, "snowTimer"), snowTimer);
    glUniform1i(glGetUniformLocation(m_shader, "sunTimer"), sunTimer);


    // Pass shape info and draw shape
    for (int i = 1; i < shapeStartIndices.size(); i++) {
    for (int i = 1; i < shapeStartIndices.size(); i++) {
        // Pass in model matrix for shape i as a uniform into the shader program
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "modelMatrix"), 1, GL_FALSE, &modelMatrixList[i][0][0]);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrixList[i])));
        glUniformMatrix3fv(glGetUniformLocation(m_shader, "normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

        // Pass in m_view and m_proj
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "viewMatrix"), 1, GL_FALSE, &m_view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "projectMatrix"), 1, GL_FALSE, &m_proj[0][0]);

        bool isShapeTexture = true;
        if (isShapeTexture) {
            glUniform1f(glGetUniformLocation(m_shader, "isTexture"), 1.0);

            if (m_image.isNull()) {
                // Handle error: Image didn't load
                glUniform1f(glGetUniformLocation(m_shader, "isTexture"), -1.0);
            }

            // Set the texture.frag uniform for our texture
            GLint textureUniform = glGetUniformLocation(m_shader, "textureImgMapping");
            glUniform1i(textureUniform, 3);  // Set the sampler uniform to use texture unit 3
        }
        else {
            glUniform1f(glGetUniformLocation(m_shader, "isTexture"), -1.0);
        }
        glUniform1f(glGetUniformLocation(m_shader, "materialBlend"), 0.4);

        // Pass shininess and world-space camera position
        glUniform1f(glGetUniformLocation(m_shader, "shininess"), renderScene.sceneMetaData.shapes[0].primitive.material.shininess);
        glm::vec4 cameraWorldSpacePos = renderScene.sceneCamera.cameraPos;
        glUniform4fv(glGetUniformLocation(m_shader, "cameraWorldSpacePos"), 1, &cameraWorldSpacePos[0]);
        // Blend Commend
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Draw Command
        glDrawArrays(GL_TRIANGLES, shapeStartIndices[i], shapeSizes[i]);
        glDisable(GL_BLEND);
    }

    // Unbind Vertex Array
    glBindVertexArray(0);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    // Deactivate the shader program by passing 0 into glUseProgram
    glUseProgram(0);
}

void Realtime::paintTerrain() {
    // Bind Vertex Data
    glBindVertexArray(m_terrain_vao);

    // Activate the shader program by calling glUseProgram with `m_terrain_shader`
    glUseProgram(m_terrain_shader);

    // ====== Pass m_ka, m_kd, m_ks into the fragment shader as a uniform
    glUniform1f(glGetUniformLocation(m_terrain_shader, "ka"), renderScene.getGlobalData().ka);
    glUniform1f(glGetUniformLocation(m_terrain_shader, "kd"), renderScene.getGlobalData().kd);
    glUniform1f(glGetUniformLocation(m_terrain_shader, "ks"), renderScene.getGlobalData().ks);

    // ====== Pass light info
    int lightCounter = 0;
    for (SceneLightData &light : renderScene.sceneMetaData.lights) {
        if (light.type == LightType::LIGHT_DIRECTIONAL) {
            GLint loc1 = glGetUniformLocation(m_terrain_shader, ("lightTypes[" + std::to_string(lightCounter) + "]").c_str());
            glUniform1f(loc1, 0);

            GLint loc2 = glGetUniformLocation(m_terrain_shader, ("lightColors[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc2, sunlightColor.x, sunlightColor.y, sunlightColor.z);

            GLint loc3 = glGetUniformLocation(m_terrain_shader, ("lightDirections[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc3, sunlightDirection.x, sunlightDirection.y, sunlightDirection.z); // rotate the sun light
        }
        if (light.type == LightType::LIGHT_POINT) {
            GLint loc1 = glGetUniformLocation(m_terrain_shader, ("lightTypes[" + std::to_string(lightCounter) + "]").c_str());
            glUniform1f(loc1, 1);

            GLint loc2 = glGetUniformLocation(m_terrain_shader, ("lightColors[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc2, light.color.x, light.color.y, light.color.z);

            GLint loc4 = glGetUniformLocation(m_terrain_shader, ("lightPositions[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc4, light.pos.x, light.pos.y, light.pos.z);

            GLint loc7 = glGetUniformLocation(m_terrain_shader, ("lightFunctions[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc7, light.function.x, light.function.y, light.function.z);
        }
        if (light.type == LightType::LIGHT_SPOT) {
            GLint loc1 = glGetUniformLocation(m_terrain_shader, ("lightTypes[" + std::to_string(lightCounter) + "]").c_str());
            glUniform1f(loc1, 2);

            GLint loc2 = glGetUniformLocation(m_terrain_shader, ("lightColors[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc2, light.color.x, light.color.y, light.color.z);

            GLint loc3 = glGetUniformLocation(m_terrain_shader, ("lightDirections[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc3, light.dir.x, light.dir.y, light.dir.z);

            GLint loc4 = glGetUniformLocation(m_terrain_shader, ("lightPositions[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc4, light.pos.x, light.pos.y, light.pos.z);

            GLint loc5 = glGetUniformLocation(m_terrain_shader, ("lightAngles[" + std::to_string(lightCounter) + "]").c_str());
            glUniform1f(loc5, light.angle);

            GLint loc6 = glGetUniformLocation(m_terrain_shader, ("lightPenumbras[" + std::to_string(lightCounter) + "]").c_str());
            glUniform1f(loc6, light.penumbra);

            GLint loc7 = glGetUniformLocation(m_terrain_shader, ("lightFunctions[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc7, light.function.x, light.function.y, light.function.z);
        }

        lightCounter ++;
        if (lightCounter >= 8) {
            break;
        }
    }

    // ====== Reset remaining lights if the current scene has fewer than 8 lights
    for (int i = lightCounter; i < 8; i++) {
        glUniform1f(glGetUniformLocation(m_terrain_shader, ("lightTypes[" + std::to_string(i) + "]").c_str()), -1); // Set to an invalid type
        glUniform3f(glGetUniformLocation(m_terrain_shader, ("lightColors[" + std::to_string(i) + "]").c_str()), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(m_terrain_shader, ("lightDirections[" + std::to_string(i) + "]").c_str()), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(m_terrain_shader, ("lightPositions[" + std::to_string(i) + "]").c_str()), 0.0f, 0.0f, 0.0f);
    }

    // ====== Pass shape info and draw shape
    // Pass in model matrix for shape i as a uniform into the shader program
    glUniformMatrix4fv(glGetUniformLocation(m_terrain_shader, "modelMatrix"), 1, GL_FALSE, &terrainModelMatrix[0][0]);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(terrainModelMatrix)));
    glUniformMatrix3fv(glGetUniformLocation(m_terrain_shader, "normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

    // ====== Pass in camera data - m_view and m_proj
    glUniformMatrix4fv(glGetUniformLocation(m_terrain_shader, "viewMatrix"), 1, GL_FALSE, &m_view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_terrain_shader, "projectMatrix"), 1, GL_FALSE, &m_proj[0][0]);
    // Pass shininess and world-space camera position
    float terrainShininess = 10;
    glUniform1f(glGetUniformLocation(m_terrain_shader, "shininess"), terrainShininess);
    glm::vec4 cameraWorldSpacePos = renderScene.sceneCamera.cameraPos;
    glUniform4fv(glGetUniformLocation(m_terrain_shader, "cameraWorldSpacePos"), 1, &cameraWorldSpacePos[0]);

    // ====== Terrain Phong settings
    glm::vec4 terrainCAmbient = glm::vec4(0.2, 0.2, 0.2, 1);
    glm::vec4 terrainCDiffuse = glm::vec4(0.5, 0.5, 0.5, 1);
    glm::vec4 terrainCSpecular = glm::vec4(0.1, 0.1, 0.1, 1);
    glUniform4fv(glGetUniformLocation(m_terrain_shader, "cAmbient"), 1, &terrainCAmbient[0]);
    glUniform4fv(glGetUniformLocation(m_terrain_shader, "cDiffuse"), 1, &terrainCDiffuse[0]);
    glUniform4fv(glGetUniformLocation(m_terrain_shader, "cSpecular"), 1, &terrainCSpecular[0]);

    // ====== Terrain texture
    bool isTerrainTexture = true; // Currently hardcoded it to be true
    if (isTerrainTexture) {
        glUniform1f(glGetUniformLocation(m_terrain_shader, "isTexture"), 1.0);

        if (m_terrain_image.isNull()) {
            // Handle error: Image didn't load
            glUniform1f(glGetUniformLocation(m_terrain_shader, "isTexture"), -1.0);
        }

        // Set the texture.frag uniform for our texture
        GLint textureUniform = glGetUniformLocation(m_terrain_shader, "textureImgMapping");
        glUniform1i(textureUniform, 1);  // Set the sampler uniform to use texture unit 1
    }
    else {
        glUniform1f(glGetUniformLocation(m_terrain_shader, "isTexture"), -1.0);
    }
    float terrainMaterialBlend = 0.5;
    glUniform1f(glGetUniformLocation(m_terrain_shader, "materialBlend"), terrainMaterialBlend);

    // ====== Pass collision map as a texture
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, m_collision_texture);
    // Update collision map
    updateTerrainCollisionMap();
    // Upload the data to the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 100, 100, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, matrixData.data());
    // Set the texture.frag uniform for our texture
    GLint textureUniform = glGetUniformLocation(m_terrain_shader, "textureCollisionMapping");
    glUniform1i(textureUniform, 2);  // Set the sampler uniform to use texture unit 2

    if (settings.increase) {
        glUniform1f(glGetUniformLocation(m_terrain_shader, "isIncrease"), 1.0f);
    }
    else {
        glUniform1f(glGetUniformLocation(m_terrain_shader, "isIncrease"), 0.0f);
    }
    glUniform1f(glGetUniformLocation(m_terrain_shader, "accumulateRate"), accumulateRate);

    // ====== Accumulation timers
    glUniform1i(glGetUniformLocation(m_terrain_shader, "snowTimer"), snowTimer);
    glUniform1i(glGetUniformLocation(m_terrain_shader, "sunTimer"), sunTimer);

    // Draw Command
    glDrawArrays(GL_TRIANGLES, terrainStartIndex, terrainSize);

    // Unbind Vertex Array
    glBindVertexArray(0);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    // Deactivate the shader program by passing 0 into glUseProgram
    glUseProgram(0);
}

void Realtime::updateSunlight(glm::vec4 originalDirection) {
    float angleRadians = glm::radians(static_cast<float>(timeTracker) * rotationSpeedScale);
    sunlightDirection = glm::rotate(angleRadians, glm::vec3(0.0f, 0.0f, 1.0f)) * originalDirection;

    // Normalize the light direction
    glm::vec3 normalizedLightDirection = glm::normalize(sunlightDirection);
    // Dot product with the y-axis
    float dotProduct = glm::dot(normalizedLightDirection, glm::vec3(0.0f, 1.0f, 0.0f));
    // Use the absolute value of the dot product to ensure we don't get negative values
    float blendFactor = glm::abs(dotProduct);
    // Define the warm and white colors
    glm::vec3 warmColor = glm::vec3(1.0f, 0.5f, 0.1f); // Warm color
    glm::vec3 whiteColor = glm::vec3(1.0f, 1.0f, 1.0f); // White color
    // Interpolate between the warm color and the white color based on the angle with the y-axis
    sunlightColor = glm::mix(warmColor, whiteColor, blendFactor);
}

void Realtime::updateTerrainCollisionMap() {
    if (settings.snow) {
        for (auto &particle : particles->getParticles()) {
            float x = particle.position.x;
            float y = particle.position.y;
            float z = particle.position.z;

            if (y >= 2.5) {
                continue;
            }

            float terrainHeight = terrainGenerator.getHeight(x, 1-z, settings.bumpiness);
            float accumulateHeight;
            if (settings.increase) {
                if (x >=0 && x <= 1 && z >=0 && z <= 1) {
                    int row = z * 100;
                    int col = x * 100;
                    int accumulateIdx = row * 100 + col;
                    accumulateHeight = matrixData[accumulateIdx] * accumulateRate;
                    terrainHeight += accumulateHeight;
                }
            }

            if (y <= terrainHeight) {
                if (x >=0 && x <= 1 && z >=0 && z <= 1) {
                    // Add shape info to static list
                    if (settings.accumulate) {
                        QString temp_imagePath = "/scenefiles/textures/snowflake.png";
                        Square squareShape;
                        squareShape.updateParams(true, 1, 1, temp_imagePath);
                        staticShapeDataList.push_back(squareShape.generateShape());
                        particle.position.y = terrainHeight + 0.001;
                        if (settings.increase) {
                            particle.position.y += accumulateHeight;
                        }
                        staticMatrixList.push_back(particles->getParticleModelMatrix(&particle));
                        staticParticleNum ++;
                    }

                    // Kill this particle
                    particle.grounded = true;

                    // TODO: May need to replace 100 with actual resolution
                    int row = z * 100;
                    int col = x * 100;
                    int accumulateIdx = row * 100 + col;
                    matrixData[accumulateIdx]++;
                }
            }

            if (y < -1) {
                // Kill this particle
                particle.grounded = true;
            }
        }
    }
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;

    // Regenerate FBOs
    makeFBO();

    shapeDataList.clear();
    vboData.clear();
    shapeStartIndices.clear();
    shapeSizes.clear();
    m_view = glm::mat4(1.0f);
    m_proj = glm::mat4(1.0f);

    if (!settings.sceneFilePath.empty()) {
        // Create scene
        renderScene = RenderScene(size().width() * m_devicePixelRatio,
                                  size().height() * m_devicePixelRatio,
                                  settings.nearPlane,
                                  settings.farPlane,
                                  metaData);
        setupShapesGL();
        setupTerrainGL();

        // Setup camera data from the scene
        m_view = renderScene.sceneCamera.getViewMatrix();
        m_proj = renderScene.sceneCamera.getProjectMatrix();
    }
}

// ********************************* Scene and settings *********************************
void Realtime::sceneChanged() {
    makeCurrent();
    resetScene();
    initializeGL();

    // Load and parse scene file
    bool success = SceneParser::parse(settings.sceneFilePath, metaData);
    if (!success) {
        std::cerr << "Error loading scene: \"" << settings.sceneFilePath << "\"" << std::endl;
    }

    // Create scene
    renderScene = RenderScene(size().width() * m_devicePixelRatio,
                              size().height() * m_devicePixelRatio,
                              settings.nearPlane,
                              settings.farPlane,
                              metaData);

    setupShapesGL();
    setupTerrainGL();

    // Setup camera data from the scene
    m_view = renderScene.sceneCamera.getViewMatrix();
    m_proj = renderScene.sceneCamera.getProjectMatrix();

    update(); // asks for a PaintGL() call to occur
}

void Realtime::settingsChanged() {
    int oldNum=particles->getParticleNum();
//    int newNum=int(1000*((1.0f*settings.intensity)/100.f));
    int newNum=settings.intensity;
    bool flagIntensity=std::abs(oldNum-newNum)>100?true:false;
    if (flagIntensity) {
        shapeDataList.clear();
        vboData.clear();
        shapeStartIndices.clear();
        shapeSizes.clear();

        if (!settings.sceneFilePath.empty()) {
            if (flagIntensity) {
                particles->updateNum(newNum);
                particles->updateSpeed();
                setupShapesGL();
            }
        }
    }

    if (settings.bumpiness != shapeParameter1Saved) {
        staticParticleNum = 0;
        staticShapeDataList.clear();
        staticMatrixList.clear();

        m_view = glm::mat4(1.0f);
        m_proj = glm::mat4(1.0f);

        if (!settings.sceneFilePath.empty()) {
            // Create scene
            renderScene = RenderScene(size().width() * m_devicePixelRatio,
                                      size().height() * m_devicePixelRatio,
                                      settings.nearPlane,
                                      settings.farPlane,
                                      metaData);
            setupTerrainGL();

            // Setup camera data from the scene
            m_view = renderScene.sceneCamera.getViewMatrix();
            m_proj = renderScene.sceneCamera.getProjectMatrix();
        }

        shapeParameter1Saved = settings.bumpiness;
    }

    particles->updateSpeed();

    if (settings.sun != isSunMove) {
        isSunMove = settings.sun;
        timeTracker = 0;
        sunlightOriginalDirection = glm::vec4(sunlightDirection, 0.0f);
    }

    if (settings.time != dayTimeSaved) {
        dayTimeSaved = settings.time;
        timeTracker = 0;
        setSunlightDirectionAccordingToTime();
    }

    update(); // asks for a PaintGL() call to occur
}

void Realtime::setSunlightDirectionAccordingToTime() {
    // Calculate the angle based on the time of day, with 0 degrees at 6AM and 180 degrees at 6PM
    float hoursSince6AM = static_cast<float>(settings.time) - 6;
    float angleDegrees = hoursSince6AM * 15.0f;

    // Adjust the angle for a full 24-hour cycle
    if (angleDegrees < 0) {
        angleDegrees += 360.0f; // Normalize the angle for times between 12AM and 6AM
    } else if (angleDegrees >= 360.0f) {
        angleDegrees -= 360.0f; // Normalize the angle for times between 6PM and 12AM
    }

    // Convert to radians
    float angleRadians = glm::radians(angleDegrees);

    // Rotate around the Z-axis to simulate the sun's path
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angleRadians, glm::vec3(0.0f, 0.0f, 1.0f));
    sunlightOriginalDirection = rotationMatrix * glm::vec4(-1.0f, 0.0f, 0.0f, 1.0f); // Original direction as vec4
    sunlightDirection = sunlightOriginalDirection;
}

void Realtime::setupShapesGL() {
    setupShapeData();

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    // Send data to VBO
    glBufferData(GL_ARRAY_BUFFER, vboData.size() * sizeof(GLfloat), vboData.data(), GL_STATIC_DRAW);
    // Bind VAO
    glBindVertexArray(m_vao);
    // Enable and define attribute 0 to store vertex positions and attribute 1 to store vertex normals
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)(0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)(6 * sizeof(GLfloat)));
    // Clean-up bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void Realtime::setupTerrainGL() {
    setupTerrainData();

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_terrain_vbo);
    // Send data to VBO
    glBufferData(GL_ARRAY_BUFFER, terrainVboData.size() * sizeof(GLfloat), terrainVboData.data(), GL_STATIC_DRAW);
    // Bind VAO
    glBindVertexArray(m_terrain_vao);
    // Enable and define attribute 0 to store vertex positions, attribute 1 to store vertex normals,  attribute 2 to store uv coordinates for textures
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)(0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)(6 * sizeof(GLfloat)));
    // Clean-up bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}

void Realtime::setupShapeData() {
    shapeDataList.clear();
    modelMatrixList.clear();
    vboData.clear();
    shapeSizes.clear();
    shapeStartIndices.clear();
    int shapeParameter1 = settings.bumpiness;
    int shapeParameter2 = settings.shapeParameter2;

    // Adaptive level of detail for object complexity
    if (settings.extraCredit1) {
        float complexityFactor = 1.0;
        if (renderScene.sceneMetaData.shapes.size() > 10) {
            complexityFactor = 1.0 / (log(0.1 * (renderScene.sceneMetaData.shapes.size() - 10) + 1) + 1);
            // complexityFactor = 0.5;
            shapeParameter1 = int(shapeParameter1 * complexityFactor);
            shapeParameter2 = int(shapeParameter2 * complexityFactor);
        }
    }

    // Set the lower bound of shape parameters
    shapeParameter1 = int(std::max(1, shapeParameter1));
    shapeParameter2 = int(std::max(3, shapeParameter2));

    // Adaptive level of detail for object distances
    std::vector<float> distanceFactors;
    if (settings.extraCredit2) {
        distanceFactors = calculateDistanceFactors();
    }

    // Setup shape data (vertices and normals) from the scene
    int shapeIdx = 0;
    for (RenderShapeData &shape : renderScene.sceneMetaData.shapes) {
        int finalShapeParameter1 = shapeParameter1;
        int finalShapeParameter2 = shapeParameter2;
        if (settings.extraCredit2) {
            finalShapeParameter1 = int(std::max(3.0f, shapeParameter1 * distanceFactors[shapeIdx]));
            finalShapeParameter2 = int(std::max(3.0f, shapeParameter2 * distanceFactors[shapeIdx]));
        }

        if (shape.primitive.type == PrimitiveType::PRIMITIVE_CUBE) {
            Cube cubeShape;
            cubeShape.updateParams(finalShapeParameter1, shape.primitive.material.textureMap.isUsed, shape.primitive.material.textureMap.repeatU, shape.primitive.material.textureMap.repeatV, QString::fromStdString(shape.primitive.material.textureMap.filename));
            shapeDataList.push_back(cubeShape.generateShape());
        }
        if (shape.primitive.type == PrimitiveType::PRIMITIVE_CONE) {
            Cone coneShape;
            coneShape.updateParams(finalShapeParameter1, finalShapeParameter2, shape.primitive.material.textureMap.isUsed, shape.primitive.material.textureMap.repeatU, shape.primitive.material.textureMap.repeatV, QString::fromStdString(shape.primitive.material.textureMap.filename));
            shapeDataList.push_back(coneShape.generateShape());
        }
        if (shape.primitive.type == PrimitiveType::PRIMITIVE_CYLINDER) {
            Cylinder cylinderShape;
            cylinderShape.updateParams(finalShapeParameter1, finalShapeParameter2, shape.primitive.material.textureMap.isUsed, shape.primitive.material.textureMap.repeatU, shape.primitive.material.textureMap.repeatV, QString::fromStdString(shape.primitive.material.textureMap.filename));
            shapeDataList.push_back(cylinderShape.generateShape());
        }
        if (shape.primitive.type == PrimitiveType::PRIMITIVE_SPHERE) {
            finalShapeParameter1 = int(std::max(2, finalShapeParameter1));
            Sphere sphereShape;
            sphereShape.updateParams(finalShapeParameter1, finalShapeParameter2, shape.primitive.material.textureMap.isUsed, shape.primitive.material.textureMap.repeatU, shape.primitive.material.textureMap.repeatV, QString::fromStdString(shape.primitive.material.textureMap.filename));
            shapeDataList.push_back(sphereShape.generateShape());
        }
        if (shape.primitive.type == PrimitiveType::PRIMITIVE_MESH) {
            Mesh mesh = loadMesh(shape.primitive.meshfile);
            shapeDataList.push_back(mesh.generateVertexData());
        }
        modelMatrixList.push_back(shape.ctm);
        shapeIdx++;
    }

    QString temp_imagePath = "/scenefiles/textures/snowflake.png";
    int particleNum = particles->getParticleNum();
    std::vector<std::vector<float>> tempShapeDataList(particleNum);
    std::vector<glm::mat4> tempModelMatrixList(particleNum);

    QVector<QFuture<void>> futures;
    for (int i = 0; i < particleNum; ++i) {
        futures.push_back(QtConcurrent::run([=, &tempShapeDataList, &tempModelMatrixList]() {
            Square squareShape;
            squareShape.updateParams(true, 1, 1, temp_imagePath);
            tempShapeDataList[i] = squareShape.generateShape();
            tempModelMatrixList[i] = particles->getModel()[i];
        }));
    }

    // 等待所有任务完成
    for (auto &future : futures) {
        future.waitForFinished();
    }

    for (int i = 0; i < particleNum; ++i) {
        shapeDataList.push_back(tempShapeDataList[i]);
        modelMatrixList.push_back(tempModelMatrixList[i]);
    }

    if (settings.accumulate) {
        for (int i = 0; i < staticParticleNum; ++i) {
            shapeDataList.push_back(staticShapeDataList[i]);
            modelMatrixList.push_back(staticMatrixList[i]);
        }
    }

    int currentIndex = 0;
    for (const auto& shapeData : shapeDataList) {
        // Record the starting index of this shape.
        shapeStartIndices.push_back(currentIndex / 8);

        // Append all the data from this shape to the VBO data.
        vboData.insert(vboData.end(), shapeData.begin(), shapeData.end());

        // Record the size (in elements) of this shape.
        shapeSizes.push_back(static_cast<int>(shapeData.size() / 8));

        // Update the current index for the next shape.
        currentIndex += shapeData.size();
    }
}

void Realtime::setupTerrainData() {
    int shapeParameter1 = settings.bumpiness;
    int shapeParameter2 = settings.shapeParameter2;

    terrainData = terrainGenerator.generateTerrain(QString::fromStdString(settings.heightMapPath), settings.bumpiness);
    terrainModelMatrix = glm::mat4(1);

    terrainVboData = terrainData;
    terrainStartIndex = 0;
    terrainSize = terrainData.size() / 8;

    matrixData = std::vector<GLuint>(100 * 100, 0);
}

std::vector<float> Realtime::calculateDistanceFactors() {
    std::vector<float> distanceFactors;
    glm::vec4 cameraWorldSpacePos = renderScene.sceneCamera.getViewMatrixInverse() * renderScene.sceneCamera.cameraPos;

    float minDistance = INFINITY;
    for (RenderShapeData &shape : renderScene.sceneMetaData.shapes) {
        glm::vec4 shapeWorldSpacePos = shape.ctm * glm::vec4(0,0,0,1);
        float distance = glm::length(cameraWorldSpacePos - shapeWorldSpacePos);
        distanceFactors.push_back(distance);
        if (distance < minDistance) {
            minDistance = distance;
        }

    }

    // Scale the distances
    if (minDistance > 0.0f) {
        for (float &factor : distanceFactors) {
            factor = 1 / (factor / minDistance);
        }
    }

    return distanceFactors;
}

void Realtime::resetScene() {
    metaData.lights.clear();
    metaData.shapes.clear();

    // Clear all the shape data.
    shapeDataList.clear();

    // Clear the model matrix list and reset each matrix to identity.
    modelMatrixList.clear();

    // Reset view and projection matrices to identity matrices.
    m_view = glm::mat4(1.0f);
    m_proj = glm::mat4(1.0f);

    // Clear VBO data and related vectors.
    vboData.clear();
    shapeStartIndices.clear();
    shapeSizes.clear();

    timeTracker = 0;
    snowTimer = 0;
    sunTimer = 0;
}

// ********************************* Key and mouse *********************************
void Realtime::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Realtime::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Realtime::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Realtime::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Realtime::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
        // Constants for mouse sensitivity (adjust as needed)
        const float sensitivityX = 0.001f; // Horizontal sensitivity
        const float sensitivityY = 0.001f; // Vertical sensitivity

        // Calculating rotation angles
        float angleX = -deltaX * sensitivityX; // Rotation angle for horizontal movement
        float angleY = -deltaY * sensitivityY; // Rotation angle for vertical movement

        // Rotation matrix around Y-axis
        glm::mat4 rotX = glm::mat4(glm::cos(angleX), 0.0f, glm::sin(angleX), 0.0f,
                                   0.0f, 1.0f, 0.0f, 0.0f,
                                   -glm::sin(angleX), 0.0f, glm::cos(angleX), 0.0f,
                                   0.0f, 0.0f, 0.0f, 1.0f);

        // Calculate right vector for rotation around arbitrary axis
        glm::vec3 right = glm::cross(glm::vec3(metaData.cameraData.look), glm::vec3(metaData.cameraData.up));
        right = glm::normalize(right);
        float cosPhi = glm::cos(angleY);
        float sinPhi = glm::sin(angleY);
        glm::mat4 rotY = glm::mat4(cosPhi + right.x * right.x * (1 - cosPhi), right.x * right.y * (1 - cosPhi) - right.z * sinPhi, right.x * right.z * (1 - cosPhi) + right.y * sinPhi, 0.0f,
                                   right.y * right.x * (1 - cosPhi) + right.z * sinPhi, cosPhi + right.y * right.y * (1 - cosPhi), right.y * right.z * (1 - cosPhi) - right.x * sinPhi, 0.0f,
                                   right.z * right.x * (1 - cosPhi) - right.y * sinPhi, right.z * right.y * (1 - cosPhi) + right.x * sinPhi, cosPhi + right.z * right.z * (1 - cosPhi), 0.0f,
                                   0.0f, 0.0f, 0.0f, 1.0f);

        // Apply rotations
        metaData.cameraData.look = rotX * metaData.cameraData.look;
        metaData.cameraData.look = rotY * metaData.cameraData.look;
        metaData.cameraData.up = rotX * metaData.cameraData.up;
        metaData.cameraData.up = rotY * metaData.cameraData.up;

        // Normalize the look and up vectors
        metaData.cameraData.look = glm::normalize(metaData.cameraData.look);
        metaData.cameraData.up = glm::normalize(metaData.cameraData.up);


        // Normalize the look and up vectors
        metaData.cameraData.look = glm::normalize(metaData.cameraData.look);
        metaData.cameraData.up = glm::normalize(metaData.cameraData.up);

        // Update scene data
        renderScene.updateCamera(settings.nearPlane, settings.farPlane, metaData);

        // Update camera data from the scene
        m_view = renderScene.sceneCamera.getViewMatrix();
        m_proj = renderScene.sceneCamera.getProjectMatrix();

        update(); // asks for a PaintGL() call to occur
    }
}

void Realtime::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
//    std::cout<<elapsedms<<std::endl;
    float deltaTime = elapsedms * 0.001f;

    if (settings.sceneFilePath!="") {
        particles->update_ParticleSystem(deltaTime);
        setupShapesGL();
        //        update_particle_vbo();

    }
    //

    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around
    if (m_keyMap[Qt::Key_W]) {
        metaData.cameraData.pos += deltaTime * metaData.cameraData.look;
    }
    if (m_keyMap[Qt::Key_S]) {
        metaData.cameraData.pos -= deltaTime * metaData.cameraData.look;
    }
    if (m_keyMap[Qt::Key_A]) {
        glm::vec3 crossProduct = glm::cross(glm::vec3(metaData.cameraData.look), glm::vec3(metaData.cameraData.up));
        metaData.cameraData.pos -= deltaTime * glm::vec4(crossProduct, 0.0f);
    }
    if (m_keyMap[Qt::Key_D]) {
        glm::vec3 crossProduct = glm::cross(glm::vec3(metaData.cameraData.look), glm::vec3(metaData.cameraData.up));
        metaData.cameraData.pos += deltaTime * glm::vec4(crossProduct, 0.0f);
    }
    if (m_keyMap[Qt::Key_Space]) {
        metaData.cameraData.pos += deltaTime * glm::vec4(0,1,0,0);
    }
    if (m_keyMap[Qt::Key_Control]) {
        metaData.cameraData.pos += deltaTime * glm::vec4(0,-1,0,0);
    }

    // Update scene data
    renderScene.updateCamera(settings.nearPlane, settings.farPlane, metaData);

    // Update camera data from the scene
    m_view = renderScene.sceneCamera.getViewMatrix();
    m_proj = renderScene.sceneCamera.getProjectMatrix();

    update(); // asks for a PaintGL() call to occur
}

// DO NOT EDIT
void Realtime::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}







