#version 330 core

out vec4 out_col;

uniform int id;

void main() {
	out_col = vec4(id / 255.0, 0.0, 0.0, 1.0);
}
