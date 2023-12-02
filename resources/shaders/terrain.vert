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
out float isAccumulate;

// Declare a uniform mat4 to store model matrix
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

// Declare uniform mat4's for the view and projection matrix
uniform mat4 viewMatrix;
uniform mat4 projectMatrix;

//uniform sampler2D textureCollisionMapping;
uniform usampler2D textureCollisionMapping;

void main() {
    // Compute the world-space position and normal, then pass them to the fragment shader
    vertexWorldSpacePos = vec3(modelMatrix * vec4(vertexObjectsSpacePos, 1.0));
    vertexWorldSpaceNormal = normalMatrix * normalize(vertexObjectSpaceNormal);
    vertexWorldSpaceNormal = normalize(vertexWorldSpaceNormal);
    textureUV = vertexTexture;

    vec2 collisionUV = vec2(vertexWorldSpacePos.x / 100.0, vertexWorldSpacePos.z / 100.0);
    ivec2 texSize = textureSize(textureCollisionMapping, 0);
    ivec2 texCoords = ivec2(collisionUV * vec2(texSize));
    uint value = texelFetch(textureCollisionMapping, texCoords, 0).r;
    if (value > 0u) {
        isAccumulate = value;
    }
    else {
        isAccumulate = -1;
    }

//    vec2 collisionUV = vec2(vertexWorldSpacePos.x / 100.0, vertexWorldSpacePos.z / 100.0);
//    ivec2 texSize = textureSize(textureCollisionMapping, 0);
//    ivec2 texCoords = ivec2(collisionUV * vec2(texSize));
//    isAccumulate = -1; // Default to -1
//     for (int i = -1; i <= 1; ++i) {
//         for (int j = -1; j <= 1; ++j) {
//             ivec2 neighborCoords = texCoords + ivec2(i, j);
//             // Ensure coordinates are within texture bounds
//             if (neighborCoords.x >= 0 && neighborCoords.x < texSize.x &&
//                 neighborCoords.y >= 0 && neighborCoords.y < texSize.y) {
//                 uint neighborValue = texelFetch(textureCollisionMapping, neighborCoords, 0).r;
//                 if (neighborValue > 0u) {
//                     isAccumulate = 1;
//                     break; // Found a neighbor with value > 0, no need to check further
//                 }
//             }
//         }
//         if (isAccumulate > 0) {
//             break; // Exit outer loop as well
//         }
//     }

    // Set gl_Position to the object space position transformed to clip space
    gl_Position = projectMatrix * viewMatrix * vec4(vertexWorldSpacePos, 1.0);
}
