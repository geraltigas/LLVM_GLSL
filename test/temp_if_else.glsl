#version 540

float time;
layout (location) uniform vec2 mouse;
layout (binding) uniform vec2 resolution;

void main(int a) {
    if (a == 0) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
}

