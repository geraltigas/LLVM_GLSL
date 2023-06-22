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
    int a = 1;
    {
        int a = 1;
        int b = 2;
    }
    {
        int a = 1;
        int b = 2;
        a = 4;
    }
    a = 2;
    return 0;
}

