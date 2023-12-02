#version 330 core
out vec4 FragColor;

uniform sampler2D particleTexture;
uniform bool useTexture;

void main() {
    vec4 particleColor = vec4(1.0, 1.0, 1.0, 1.0);

//    if (useTexture) {
//        particleColor *= texture(particleTexture, gl_PointCoord);
//    }

    FragColor = particleColor;
}
