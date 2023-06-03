#version 540

float time;
layout (location) uniform vec2 mouse;
layout (binding) uniform vec2 resolution;

void main(int a) {
    vec4 color = mix(c1, c2, pos.y / 150. * (1.)/2. *2.);
    vec4 color = mix(c1, c2, pos.y / 150. * (sin(pos.x - cos(time-pos.y+pos.x) )+1.)/2. *2.);
    vec4 c3 = mix(4);
    vec4 c1 = vec4(255, 0, 0, 255);
    vec2 pos = (gl_FragCoord.xy / resolution.xy);
    vec4 c2 = vec4(255, 255, 0, 255);
    gl_FragColor = color;
}

