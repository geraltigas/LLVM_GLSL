#version 540

float time;
double d;
bool b;
int i;
vec2 v;
vec3 v3;
vec4 v4;
mat2 m2;
mat3 m3;
mat4 m4;

int main() {
    int a = 1,1,1;
    double d = 2.0;
    bool b = 1;
    int i = 2;
    vec2 v = vec2(1.0, 2.0);
    vec3 v3 = vec3(1.0, 2.0, 3.0);
    vec4 v4 = vec4(1.0, 2.0, 3.0, 4.0);
    mat2 m2 = mat2(1.0, 2.0, 3.0, 4.0);
    mat3 m3 = mat3(1.0, 2.0, 3.0, 4.0, 5.0, 6.0,7.0,8.0,9.0);
    mat4 m4 = mat4(1.0, 2.0, 3.0, 4.0, 5.0, 6.0,7.0,8.0,9.0,10.0,11.0,12.0,13.0,14.0,15,16);
    a = 2;
    return 0;
}