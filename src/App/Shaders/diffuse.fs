#version 330 core

in vec2 vert_pos;
out vec4 out_col;

uniform int iterations;
uniform float radius;

void main() {
    float x = vert_pos.x;
    float y = vert_pos.y;
    float zx = 0;
    float zy = 0;
    for (int k = 0; k < iterations; k++) {
        float newzx = zx * zx - zy * zy + x;
        float newzy = 2 * zx * zy + y;
        zx = newzx;
        zy = newzy;
        if (zx * zx + zy * zy > radius) {
            out_col = vec4(min(1.0, 1.0 / iterations * 4 * k), 0.0, 0.0, 1.0);
            break;
        }
    }
    if (zx * zx + zy * zy <= radius) {
        out_col = vec4(1.0, 1.0, 1.0, 1.0);
    }
}
