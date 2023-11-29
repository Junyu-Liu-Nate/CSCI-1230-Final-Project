#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <iostream>

class Shape {
public:
    GLuint vao;
    GLuint vbo;
    std::vector<GLfloat> vertices;
    glm::mat4 modelMatrix;
    // Add other properties such as material properties, texture, etc.

    // Function to initialize the shape
    void initializeGL() {
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // Define the vertex data layout here
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), reinterpret_cast<void *>(0));

        // Unbind to cleanup
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Function to render the shape
    void render(GLuint shaderProgram) {
        glBindVertexArray(vao);
        glUseProgram(shaderProgram);

        // Pass transformation matrices and other uniforms here
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelMatrix"), 1, GL_FALSE, &modelMatrix[0][0]);

        // Now draw the shape
        glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);

        // Clean up
        glBindVertexArray(0);
        glUseProgram(0);
    }
};

