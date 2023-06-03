#version 540

float time;
layout (location) uniform vec2 mouse;
layout (binding) uniform vec2 resolution;

void main(int a) {
    do {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } while (true);
}

