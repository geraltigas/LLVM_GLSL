#version 540

float time;
layout (location) uniform vec2 mouse;
layout (binding) uniform vec2 resolution;

void main(int a) {
    if (a == 0) {
        gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    }
    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);
    mat2 rot = mat2(1,1,1,1);
    rot[1] = vec2(0,0);
}
