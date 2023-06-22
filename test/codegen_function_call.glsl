#version 540

int get_int() {
    return 1;
}

int get_int_(int a) {
    return a;
}

int main() {
    int a = get_int();
    int b = get_int_(2);
}

