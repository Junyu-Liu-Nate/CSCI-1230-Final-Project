#version 330 core

// Declare "in" variables for the world-space position and normal,
//         received post-interpolation from the vertex shader
in vec3 vertexWorldSpacePos;
in vec3 vertexWorldSpaceNormal;
in vec2 textureUV;

// Declare an out vec4 for your output color
out vec4 fragColor;

// Declare relevant uniform(s) here, for ambient lighting

// Declare relevant uniform(s) here, for diffuse lighting


uniform float isTexture;
uniform float materialBlend;

uniform sampler2D textureImgMapping;

// Declare relevant uniform(s) here, for specular lighting




// Timers
uniform int snowTimer;
uniform int rainTimer;
uniform int sunTimer;

void main() {
    // Need to renormalize vectors here if you want them to be normalized

    fragColor = vec4(0.0, 0.0, 0.0, 1.0);
        if (isTexture > 0) {
            vec4 textureColor = vec4(1);
            textureColor = texture(textureImgMapping, textureUV);
            fragColor.a = textureColor.a;
        }


    fragColor.x =1.0f; /*clamp(fragColor.x, 0.0f, 1.0f);*/
    fragColor.y =1.0f; /*clamp(fragColor.y, 0.0f, 1.0f);*/
    fragColor.z = 1.0f;/*clamp(fragColor.z, 0.0f, 1.0f);*/
    if (fragColor.a < 0.1)
            discard;
}
