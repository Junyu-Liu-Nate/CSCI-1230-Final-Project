#version 330 core

// Create a UV coordinate in variable
in vec2 uvCoord;

// Add a sampler2D uniform
uniform sampler2D textureImg;

// Gradient color uniforms
uniform vec3 gradientStartColor; // The start color of the gradient
uniform vec3 gradientEndColor;   // The end color of the gradient
uniform vec2 gradientDirection;  // The direction of the gradient

// Add bools on whether or not to filter the texture
uniform bool isPerPixelFilter;
uniform bool isKernelFilter;
uniform bool isFXAA;

uniform float screenWidth;
uniform float screenHeight;

out vec4 fragColor;

// Helper function to calculate luminance
float luminance(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

void main()
{
    fragColor = vec4(1);
    // Set fragColor using the sampler2D at the UV coordinate
    fragColor = texture(textureImg, uvCoord);

    // Calculate the current pixel's luminance
    float luma = luminance(fragColor.rgb);

    // Check if the pixel is black or near black
    if (luma < 0.01) { // Threshold can be adjusted as needed
        // Calculate the gradient factor based on the UV coordinate and direction
        float gradientFactor = dot(uvCoord, gradientDirection);

        // Linearly interpolate between the start and end colors based on the gradient factor
        vec3 gradientColor = mix(gradientStartColor, gradientEndColor, gradientFactor);

        // Apply the gradient color to the black pixel
        fragColor.rgb = gradientColor;
    }

    if (isFXAA) {
//        vec2 inverseScreenSize = vec2(1.0 / screenWidth, 1.0 / screenHeight);

//        // Sample the texture at the current fragment's UV coordinates
//        vec4 color = texture(textureImg, uvCoord);

//        // FXAA
//        vec4 neighbors[4];
//        neighbors[0] = texture(textureImg, uvCoord + vec2(-inverseScreenSize.x, 0.0));
//        neighbors[1] = texture(textureImg, uvCoord + vec2(inverseScreenSize.x, 0.0));
//        neighbors[2] = texture(textureImg, uvCoord + vec2(0.0, -inverseScreenSize.y));
//        neighbors[3] = texture(textureImg, uvCoord + vec2(0.0, inverseScreenSize.y));

//        float edgeThresholdMin = 0.0312;
//        float edgeThresholdMax = 0.125;
//        float luma[5];
//        luma[0] = dot(color.rgb, vec3(0.299, 0.587, 0.114));
//        for (int i = 0; i < 4; i++) {
//            luma[i + 1] = dot(neighbors[i].rgb, vec3(0.299, 0.587, 0.114));
//        }

//        float lumaMin = min(luma[0], min(min(luma[1], luma[2]), min(luma[3], luma[4])));
//        float lumaMax = max(luma[0], max(max(luma[1], luma[2]), max(luma[3], luma[4])));

//        if (lumaMax - lumaMin < max(edgeThresholdMin, lumaMax * edgeThresholdMax))
//            fragColor = color;
//        else
//            fragColor = (neighbors[0] + neighbors[1] + neighbors[2] + neighbors[3]) * 0.25;

        vec2 inverseScreenSize = vec2(1.0 / screenWidth, 1.0 / screenHeight);

                vec4 color = texture(textureImg, uvCoord);
                float lumaCenter = dot(color.rgb, vec3(0.299, 0.587, 0.114));

                // Sample neighboring pixels and compute their luminance
                vec2 offsets[8] = vec2[](
                    vec2(-1.0, -1.0), vec2(0.0, -1.0), vec2(1.0, -1.0),
                    vec2(-1.0,  0.0),                  vec2(1.0,  0.0),
                    vec2(-1.0,  1.0), vec2(0.0,  1.0), vec2(1.0,  1.0)
                );

                float lumaMin = lumaCenter;
                float lumaMax = lumaCenter;
                for (int i = 0; i < 8; ++i) {
                    float lumaNeighbor = dot(texture(textureImg, uvCoord + offsets[i] * inverseScreenSize).rgb, vec3(0.299, 0.587, 0.114));
                    lumaMin = min(lumaMin, lumaNeighbor);
                    lumaMax = max(lumaMax, lumaNeighbor);
                }

                // Determine edge strength
                float lumaRange = lumaMax - lumaMin;
                float edgeStrength = max(lumaRange / lumaMax, 0.0);

                // Sub-pixel anti-aliasing: Adjust edge strength based on local contrast
                edgeStrength *= (1.0 + 8.0 * dot(color.rgb - vec3(lumaCenter), color.rgb - vec3(lumaCenter)));

                // Edge detection thresholds
                float edgeThresholdMin = 0.0312;
                float edgeThresholdMax = 0.125;

                if (edgeStrength > max(edgeThresholdMin, lumaMax * edgeThresholdMax)) {
                    // Calculate gradient and blend factor
                    vec2 gradient = vec2(
                        lumaCenter - dot(texture(textureImg, uvCoord + vec2(-1.0, 0.0) * inverseScreenSize).rgb, vec3(0.299, 0.587, 0.114)),
                        lumaCenter - dot(texture(textureImg, uvCoord + vec2(0.0, -1.0) * inverseScreenSize).rgb, vec3(0.299, 0.587, 0.114))
                    );

                    float gradientLength = length(gradient);
                    vec2 gradientDirection = normalize(gradient) * inverseScreenSize;

                    // Sample along the gradient to find the end of the edge
                    float endLuma = dot(texture(textureImg, uvCoord + gradientDirection).rgb, vec3(0.299, 0.587, 0.114));

                    // Blend factor calculation
                    float blendFactor = max(0.0, (endLuma - lumaMin) / (lumaMax - lumaMin));
                    blendFactor = min(max(edgeStrength, 0.1), blendFactor);

                    // Blend the color
                    vec4 blendedColor = mix(color, texture(textureImg, uvCoord + gradientDirection), blendFactor);
                    fragColor = blendedColor;
                } else {
                    fragColor = color;
                }
    }

    // Per-pixel filter
    if (isPerPixelFilter) {
        // === Choice 1: Inverse filter
        fragColor.x = 1 - fragColor.x;
        fragColor.y = 1 - fragColor.y;
        fragColor.z = 1 - fragColor.z;

        // === Choice 2: Gray isKernelFilter
//        float gray = fragColor.x * 0.21 + fragColor.y * 0.72 + fragColor.z * 0.07;
//        fragColor = vec4(gray, gray, gray, 1.0);
    }

    // Kernel filter
    if (isKernelFilter) {
        // === Choice 1: 5 by 5 Box filter
        vec2 onePixel = vec2(1.0 / screenWidth, 1.0 / screenHeight);
        vec4 colorSum = vec4(0.0);
        for(int i = -2; i <= 2; i++) {
            for(int j = -2; j <= 2; j++) {
                vec2 offset = vec2(onePixel.x * float(i), onePixel.y * float(j));
                colorSum += texture(textureImg, uvCoord + offset);
            }
        }
        fragColor = colorSum / 25.0;

        // === Choice 2: Two stage sobel filter
//        vec2 onePixel = vec2(1.0 / screenWidth, 1.0 / screenHeight);
//        vec3 Gx = vec3(0);
//        vec3 Gy = vec3(0);

//        // Offset array for surrounding pixels in a 3x3 kernel
//        int[3] offset = int[](-1, 0, 1);

//        // Sample the surrounding pixels using the Sobel kernels
//        for (int i = 0; i < 3; i++) {
//            for (int j = 0; j < 3; j++) {
//                vec2 sampleUV = uvCoord + onePixel * vec2(float(offset[j]), float(offset[i]));
//                vec4 sampleColor = texture(textureImg, sampleUV);
//                float sampleIntensity = sampleColor.x * 0.21 + sampleColor.y * 0.72 + sampleColor.z * 0.07;

//                // Apply the Gx kernel
//                if (i == 0) Gx += sampleIntensity * vec3(-2, 0, 2);
//                else Gx += sampleIntensity * vec3(float(offset[j]), 0.0, -float(offset[j]));

//                // Apply the Gy kernel
//                if (j == 0) Gy += sampleIntensity * vec3(-2, 0, 2);
//                else Gy += sampleIntensity * vec3(float(offset[i]), 0.0, -float(offset[i]));
//            }
//        }

//        // Calculate the gradient magnitude
//        float G = length(vec2(dot(Gx, Gx), dot(Gy, Gy)));
//        fragColor = vec4(G, G, G, 1.0);
    }

    // When two filters are both activated
    if (isKernelFilter && isPerPixelFilter) {
        vec2 onePixel = vec2(1.0 / screenWidth, 1.0 / screenHeight); // Assuming a 300x300 texture
        vec4 colorSum = vec4(0.0);

        for(int i = -2; i <= 2; i++) {
            for(int j = -2; j <= 2; j++) {
                vec2 offset = vec2(onePixel.x * float(i), onePixel.y * float(j));
                colorSum += texture(textureImg, uvCoord + offset);
            }
        }

        fragColor = colorSum / 25.0;

        fragColor.x = 1 - fragColor.x;
        fragColor.y = 1 - fragColor.y;
        fragColor.z = 1 - fragColor.z;
    }
}

