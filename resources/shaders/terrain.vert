#version 330 core

// Declare a vec3 object-space position variable, using
//         the `layout` and `in` keywords.
layout(location = 0) in vec3 vertexObjectsSpacePos;
layout(location = 1) in vec3 vertexObjectSpaceNormal;
layout(location = 2) in vec2 vertexTexture;

// Declare `out` variables for the world-space position and normal,
//         to be passed to the fragment shader
out vec3 vertexWorldSpacePos;
out vec3 vertexWorldSpaceNormal;
out vec2 textureUV;

// Declare a uniform mat4 to store model matrix
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

// Declare uniform mat4's for the view and projection matrix
uniform mat4 viewMatrix;
uniform mat4 projectMatrix;

void main() {
    // Compute the world-space position and normal, then pass them to the fragment shader
    vertexWorldSpacePos = vec3(modelMatrix * vec4(vertexObjectsSpacePos, 1.0));
    vertexWorldSpaceNormal = normalMatrix * normalize(vertexObjectSpaceNormal);
    vertexWorldSpaceNormal = normalize(vertexWorldSpaceNormal);
    textureUV = vertexTexture;

    // Set gl_Position to the object space position transformed to clip space
    gl_Position = projectMatrix * viewMatrix * vec4(vertexWorldSpacePos, 1.0);
}
