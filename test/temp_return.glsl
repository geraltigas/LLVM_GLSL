#version 540

float time;
layout (location) uniform vec2 mouse;
layout (binding) uniform vec2 resolution;

int main(int a) {
    while (true) {
        time += 0.01;
        vec2 uv = gl_FragCoord.xy / resolution.xy;
        vec3 col = 0.5 + 0.5 * cos(time + uv.xyx + vec3(0, 2, 4));
        gl_FragColor = vec4(col, 1.0);
        if (gl_FragColor.r > 0.5) {
            break;
        }else {
            continue;
        }
        if (gl_FragColor.r > 0.5) {
            return 1;
        }
    }
    return;
}

