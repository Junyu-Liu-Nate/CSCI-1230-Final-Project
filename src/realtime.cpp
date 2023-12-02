#include "realtime.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include <QDir>
#include <QDebug>
#include "settings.h"

#include "utils/shaderloader.h"
#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "settings.h"
#include "utils/sceneparser.h"

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

    // If you must use this function, do not edit anything above this
}

void Realtime::finish() {
    killTimer(m_timer);
    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here
    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

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

    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();

    // Initializing GL.
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

    // Load the main(default) shader
    m_shader = ShaderLoader::createShaderProgram("resources/shaders/default.vert", "resources/shaders/default.frag");
    // Generate VBO
    glGenBuffers(1, &m_vbo);
    // Generate VAO
    glGenVertexArrays(1, &m_vao);
    // Generate texture image
    glGenTextures(1, &m_texture);

    // Generate Terrain-related stuff
    m_terrain_shader = ShaderLoader::createShaderProgram("resources/shaders/terrain.vert", "resources/shaders/terrain.frag");
    // Generate terrain VBO
    glGenBuffers(1, &m_terrain_vbo);
    // Generate terrain VAO
    glGenVertexArrays(1, &m_terrain_vao);
    // Generate texture image
    glGenTextures(1, &m_terrain_texture);
    matrixData = std::vector<GLuint>(100 * 100, 0);

    // Texture shader - operates on FBO
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
    if (settings.rain) {rainTimer += 1;}
    if (settings.sun) {sunTimer += 1;}

    // Bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Call glViewport
    glViewport(0, 0, m_screen_width, m_screen_height);
    // Clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ====== Draw with default shader
    paintGeometry();

    // ====== Draw with terrain shader
    paintTerrain();

    // ====== Draw with frame shader
    // Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width, m_screen_height);
    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Call paintFrame to draw our FBO color attachment texture
    paintFrame(m_fbo_texture);
}

void Realtime::paintFrame(GLuint texture) {
    glUseProgram(m_frame_shader);

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
    // Bind Vertex Data
    glBindVertexArray(m_vao);

    // Activate the shader program by calling glUseProgram with `m_shader`
    glUseProgram(m_shader);

    glUniform1i(glGetUniformLocation(m_shader, "snowTimer"), snowTimer);
    glUniform1i(glGetUniformLocation(m_shader, "rainTimer"), rainTimer);
    glUniform1i(glGetUniformLocation(m_shader, "sunTimer"), sunTimer);

    // Pass m_ka, m_kd, m_ks into the fragment shader as a uniform
    glUniform1f(glGetUniformLocation(m_shader, "ka"), renderScene.getGlobalData().ka);
    glUniform1f(glGetUniformLocation(m_shader, "kd"), renderScene.getGlobalData().kd);
    glUniform1f(glGetUniformLocation(m_shader, "ks"), renderScene.getGlobalData().ks);

    // Pass light info
    int lightCounter = 0;
    for (SceneLightData &light : renderScene.sceneMetaData.lights) {
        if (light.type == LightType::LIGHT_DIRECTIONAL) {
            GLint loc1 = glGetUniformLocation(m_shader, ("lightTypes[" + std::to_string(lightCounter) + "]").c_str());
            glUniform1f(loc1, 0);

            GLint loc2 = glGetUniformLocation(m_shader, ("lightColors[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc2, light.color.x, light.color.y, light.color.z);

            GLint loc3 = glGetUniformLocation(m_shader, ("lightDirections[" + std::to_string(lightCounter) + "]").c_str());
//            float angleRadians = glm::radians(static_cast<float>(timeTracker));
//            glm::vec3 rotatedLightDirection = glm::rotate(angleRadians, glm::vec3(0.0f, 0.0f, 1.0f)) * light.dir;
//            glUniform3f(loc3, rotatedLightDirection.x, rotatedLightDirection.y, rotatedLightDirection.z); // rotate the sun light
            glUniform3f(loc3, light.dir.x, light.dir.y, light.dir.z);
        }
        if (light.type == LightType::LIGHT_POINT) {
            GLint loc1 = glGetUniformLocation(m_shader, ("lightTypes[" + std::to_string(lightCounter) + "]").c_str());
            glUniform1f(loc1, 1);

            GLint loc2 = glGetUniformLocation(m_shader, ("lightColors[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc2, light.color.x, light.color.y, light.color.z);

            GLint loc4 = glGetUniformLocation(m_shader, ("lightPositions[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc4, light.pos.x, light.pos.y, light.pos.z);

            GLint loc7 = glGetUniformLocation(m_shader, ("lightFunctions[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc7, light.function.x, light.function.y, light.function.z);
        }
        if (light.type == LightType::LIGHT_SPOT) {
            GLint loc1 = glGetUniformLocation(m_shader, ("lightTypes[" + std::to_string(lightCounter) + "]").c_str());
            glUniform1f(loc1, 2);

            GLint loc2 = glGetUniformLocation(m_shader, ("lightColors[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc2, light.color.x, light.color.y, light.color.z);

            GLint loc3 = glGetUniformLocation(m_shader, ("lightDirections[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc3, light.dir.x, light.dir.y, light.dir.z);

            GLint loc4 = glGetUniformLocation(m_shader, ("lightPositions[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc4, light.pos.x, light.pos.y, light.pos.z);

            GLint loc5 = glGetUniformLocation(m_shader, ("lightAngles[" + std::to_string(lightCounter) + "]").c_str());
            glUniform1f(loc5, light.angle);

            GLint loc6 = glGetUniformLocation(m_shader, ("lightPenumbras[" + std::to_string(lightCounter) + "]").c_str());
            glUniform1f(loc6, light.penumbra);

            GLint loc7 = glGetUniformLocation(m_shader, ("lightFunctions[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc7, light.function.x, light.function.y, light.function.z);
        }

        lightCounter ++;
        if (lightCounter >= 8) {
            break;
        }
    }

    // Reset remaining lights if the current scene has fewer than 8 lights
    for (int i = lightCounter; i < 8; i++) {
        glUniform1f(glGetUniformLocation(m_shader, ("lightTypes[" + std::to_string(i) + "]").c_str()), -1); // Set to an invalid type
        glUniform3f(glGetUniformLocation(m_shader, ("lightColors[" + std::to_string(i) + "]").c_str()), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(m_shader, ("lightDirections[" + std::to_string(i) + "]").c_str()), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(m_shader, ("lightPositions[" + std::to_string(i) + "]").c_str()), 0.0f, 0.0f, 0.0f);
    }

    // Pass shape info and draw shape
    for (int i = 0; i < shapeStartIndices.size(); i++) {
        // Pass in model matrix for shape i as a uniform into the shader program
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "modelMatrix"), 1, GL_FALSE, &modelMatrixList[i][0][0]);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrixList[i])));
        glUniformMatrix3fv(glGetUniformLocation(m_shader, "normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

        // Pass in m_view and m_proj
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "viewMatrix"), 1, GL_FALSE, &m_view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(m_shader, "projectMatrix"), 1, GL_FALSE, &m_proj[0][0]);

        glUniform4fv(glGetUniformLocation(m_shader, "cAmbient"), 1, &renderScene.sceneMetaData.shapes[i].primitive.material.cAmbient[0]);
        glUniform4fv(glGetUniformLocation(m_shader, "cDiffuse"), 1, &renderScene.sceneMetaData.shapes[i].primitive.material.cDiffuse[0]);
        glUniform4fv(glGetUniformLocation(m_shader, "cSpecular"), 1, &renderScene.sceneMetaData.shapes[i].primitive.material.cSpecular[0]);

        if (renderScene.sceneMetaData.shapes.at(i).primitive.material.textureMap.isUsed) {
            glUniform1f(glGetUniformLocation(m_shader, "isTexture"), 1.0);

            // Prepare texture image filepath
//            QString texture_filepath = QString::fromStdString(renderScene.sceneMetaData.shapes.at(i).primitive.material.textureMap.filename);
            texture_filepath_saved = ""; // Currently hardcoded to fix - this texture for snow should be fixed anyway
            QString currentDir = QDir::currentPath();
            QString texture_filepath = currentDir + QString::fromStdString("/scenefiles/textures/bark.png");
            // Only load texture image when texture image is changed
            if (!(texture_filepath == texture_filepath_saved)) {
                // Obtain image from filepath
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
            }
            if (m_image.isNull()) {
                // Handle error: Image didn't load
                glUniform1f(glGetUniformLocation(m_shader, "isTexture"), -1.0);
            }
            glGenTextures(1, &m_texture);
            // Set the active texture slot to texture slot 1
            glActiveTexture(GL_TEXTURE1);
            // Bind texture
            glBindTexture(GL_TEXTURE_2D, m_texture);
            // Load image into texture
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.width(), m_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());
            // Set min and mag filters' interpolation mode to linear
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Set the texture.frag uniform for our texture
            GLint textureUniform = glGetUniformLocation(m_shader, "textureImgMapping");
            glUniform1i(textureUniform, 1);  // Set the sampler uniform to use texture unit 1
        }
        else {
            glUniform1f(glGetUniformLocation(m_shader, "isTexture"), -1.0);
        }
        glUniform1f(glGetUniformLocation(m_shader, "materialBlend"), renderScene.sceneMetaData.shapes.at(i).primitive.material.blend);

        // Pass shininess and world-space camera position
        glUniform1f(glGetUniformLocation(m_shader, "shininess"), renderScene.sceneMetaData.shapes[i].primitive.material.shininess);
        glm::vec4 cameraWorldSpacePos = renderScene.sceneCamera.cameraPos;
        glUniform4fv(glGetUniformLocation(m_shader, "cameraWorldSpacePos"), 1, &cameraWorldSpacePos[0]);

        // Draw Command
        glDrawArrays(GL_TRIANGLES, shapeStartIndices[i], shapeSizes[i]);
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

    // Activate the shader program by calling glUseProgram with `m_shader`
    glUseProgram(m_terrain_shader);

    glUniform1i(glGetUniformLocation(m_terrain_shader, "snowTimer"), snowTimer);
    glUniform1i(glGetUniformLocation(m_terrain_shader, "rainTimer"), rainTimer);
    glUniform1i(glGetUniformLocation(m_terrain_shader, "sunTimer"), sunTimer);

    // Pass m_ka, m_kd, m_ks into the fragment shader as a uniform
    glUniform1f(glGetUniformLocation(m_terrain_shader, "ka"), renderScene.getGlobalData().ka);
    glUniform1f(glGetUniformLocation(m_terrain_shader, "kd"), renderScene.getGlobalData().kd);
    glUniform1f(glGetUniformLocation(m_terrain_shader, "ks"), renderScene.getGlobalData().ks);

    // Pass light info
    int lightCounter = 0;
    for (SceneLightData &light : renderScene.sceneMetaData.lights) {
        if (light.type == LightType::LIGHT_DIRECTIONAL) {
            GLint loc1 = glGetUniformLocation(m_terrain_shader, ("lightTypes[" + std::to_string(lightCounter) + "]").c_str());
            glUniform1f(loc1, 0);

            GLint loc2 = glGetUniformLocation(m_terrain_shader, ("lightColors[" + std::to_string(lightCounter) + "]").c_str());
            glUniform3f(loc2, light.color.x, light.color.y, light.color.z);

            GLint loc3 = glGetUniformLocation(m_terrain_shader, ("lightDirections[" + std::to_string(lightCounter) + "]").c_str());
//            float angleRadians = glm::radians(static_cast<float>(timeTracker));
//            glm::vec3 rotatedLightDirection = glm::rotate(angleRadians, glm::vec3(0.0f, 0.0f, 1.0f)) * light.dir;
//            glUniform3f(loc3, rotatedLightDirection.x, rotatedLightDirection.y, rotatedLightDirection.z); // rotate the sun light
            glUniform3f(loc3, light.dir.x, light.dir.y, light.dir.z);
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

    // Reset remaining lights if the current scene has fewer than 8 lights
    for (int i = lightCounter; i < 8; i++) {
        glUniform1f(glGetUniformLocation(m_terrain_shader, ("lightTypes[" + std::to_string(i) + "]").c_str()), -1); // Set to an invalid type
        glUniform3f(glGetUniformLocation(m_terrain_shader, ("lightColors[" + std::to_string(i) + "]").c_str()), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(m_terrain_shader, ("lightDirections[" + std::to_string(i) + "]").c_str()), 0.0f, 0.0f, 0.0f);
        glUniform3f(glGetUniformLocation(m_terrain_shader, ("lightPositions[" + std::to_string(i) + "]").c_str()), 0.0f, 0.0f, 0.0f);
    }

    // Pass shape info and draw shape
    // Pass in model matrix for shape i as a uniform into the shader program
    glUniformMatrix4fv(glGetUniformLocation(m_terrain_shader, "modelMatrix"), 1, GL_FALSE, &terrainModelMatrix[0][0]);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(terrainModelMatrix)));
    glUniformMatrix3fv(glGetUniformLocation(m_terrain_shader, "normalMatrix"), 1, GL_FALSE, &normalMatrix[0][0]);

    // Pass in m_view and m_proj
    glUniformMatrix4fv(glGetUniformLocation(m_terrain_shader, "viewMatrix"), 1, GL_FALSE, &m_view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_terrain_shader, "projectMatrix"), 1, GL_FALSE, &m_proj[0][0]);

    glm::vec4 terrainCAmbient = glm::vec4(0.2, 0.2, 0.2, 1);
    glm::vec4 terrainCDiffuse = glm::vec4(0.5, 0.5, 0.5, 1);
    glm::vec4 terrainCSpecular = glm::vec4(0.1, 0.1, 0.1, 1);
    glUniform4fv(glGetUniformLocation(m_terrain_shader, "cAmbient"), 1, &terrainCAmbient[0]);
    glUniform4fv(glGetUniformLocation(m_terrain_shader, "cDiffuse"), 1, &terrainCDiffuse[0]);
    glUniform4fv(glGetUniformLocation(m_terrain_shader, "cSpecular"), 1, &terrainCSpecular[0]);

    bool isTerrainTexture = true; // Currently hardcoded it to be false
    if (isTerrainTexture) {
        glUniform1f(glGetUniformLocation(m_terrain_shader, "isTexture"), 1.0);

        // Prepare texture image filepath
        texture_filepath_saved = "";
        QString currentDir = QDir::currentPath();
        QString texture_filepath = currentDir + QString::fromStdString("/scenefiles/textures/bark.png");
        // Only load texture image when texture image is changed
        if (!(texture_filepath == texture_filepath_saved)) {
            // Obtain image from filepath
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
        }
        if (m_terrain_image.isNull()) {
            // Handle error: Image didn't load
            glUniform1f(glGetUniformLocation(m_terrain_shader, "isTexture"), -1.0);
        }
        glGenTextures(1, &m_terrain_texture);
        // Set the active texture slot to texture slot 1
        glActiveTexture(GL_TEXTURE1);
        // Bind texture
        glBindTexture(GL_TEXTURE_2D, m_terrain_texture);
        // Load image into texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_terrain_image.width(), m_terrain_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_terrain_image.bits());
        // Set min and mag filters' interpolation mode to linear
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Set the texture.frag uniform for our texture
        GLint textureUniform = glGetUniformLocation(m_terrain_shader, "textureImgMapping");
        glUniform1i(textureUniform, 1);  // Set the sampler uniform to use texture unit 1
    }
    else {
        glUniform1f(glGetUniformLocation(m_terrain_shader, "isTexture"), -1.0);
    }
    float terrainMaterialBlend = 0.5;
    glUniform1f(glGetUniformLocation(m_terrain_shader, "materialBlend"), terrainMaterialBlend);

    // Pass shininess and world-space camera position
    float terrainShininess = 10;
    glUniform1f(glGetUniformLocation(m_terrain_shader, "shininess"), terrainShininess);
    glm::vec4 cameraWorldSpacePos = renderScene.sceneCamera.cameraPos;
    glUniform4fv(glGetUniformLocation(m_terrain_shader, "cameraWorldSpacePos"), 1, &cameraWorldSpacePos[0]);

    // Pass collision map as a texture
    glGenTextures(1, &m_collision_texture);
    // Set the active texture slot to texture slot 2
    glActiveTexture(GL_TEXTURE2);
    // Bind texture
    glBindTexture(GL_TEXTURE_2D, m_collision_texture);
//    // Generate random integer data for the texture
//    std::vector<GLuint> matrixData(100 * 100);
//    for (auto& val : matrixData) {
//        val = static_cast<GLuint>(rand() % 2); // Random value either 0 or 1
//    }
    for (int i = 0; i < 100; ++i) { // Update 10 random positions per frame
        int randomIndex = rand() % (100 * 100);
        matrixData[randomIndex]++;
    }
    // Upload the data to the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 100, 100, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, matrixData.data());
    // Set min and mag filters' interpolation mode to nearest
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Set the texture.frag uniform for our texture
    GLint textureUniform = glGetUniformLocation(m_terrain_shader, "textureCollisionMapping");
    glUniform1i(textureUniform, 2);  // Set the sampler uniform to use texture unit 1

    // Draw Command
    glDrawArrays(GL_TRIANGLES, terrainStartIndex, terrainSize);

    // Unbind Vertex Array
    glBindVertexArray(0);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    // Deactivate the shader program by passing 0 into glUseProgram
    glUseProgram(0);
}

void Realtime::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

    // Students: anything requiring OpenGL calls when the program starts should be done here
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
    if (settings.shapeParameter1 != shapeParameter1Saved || settings.shapeParameter2 != shapeParameter2Saved) {
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

        shapeParameter1Saved = settings.shapeParameter1;
        shapeParameter2Saved = settings.shapeParameter2;
    }

    update(); // asks for a PaintGL() call to occur
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
    int shapeParameter1 = settings.shapeParameter1;
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
    int shapeParameter1 = settings.shapeParameter1;
    int shapeParameter2 = settings.shapeParameter2;

    // Set the lower bound of shape parameters
    shapeParameter1 = int(std::max(1, shapeParameter1));
    shapeParameter2 = int(std::max(3, shapeParameter2));

    TerrainGenerator testTerrain;
    terrainData = testTerrain.generateTerrain();
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
    rainTimer = 0;
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
    float deltaTime = elapsedms * 0.001f;
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
