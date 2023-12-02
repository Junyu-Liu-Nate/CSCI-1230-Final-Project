#version 330 core
layout(location=0) in vec3 particle_pos;
uniform mat4 projectMatrix; //
uniform float pointSize;
void main(void)
{
    gl_Position = projectMatrix * vec4(particle_pos, 1.0);
    gl_PointSize = pointSize;
}
