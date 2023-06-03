#version 540

float time;
layout (location) uniform vec2 mouse;
layout (binding) uniform vec2 resolution;

void main(int a) {
    for (int i = 0; i < 1000000; i++) {
        float x = float(i) / 1000000.0;
        float y = float(i) / 1000000.0;
        float z = float(i) / 1000000.0;
    }
}

