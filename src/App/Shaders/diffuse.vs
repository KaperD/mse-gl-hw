#version 330 core

layout(location=0) in vec2 pos;

uniform vec2 shift;
uniform float zoom_scale;

out vec2 vert_pos;

void main() {
    vert_pos = vec2((pos.x + shift.x) / zoom_scale, (pos.y + shift.y) / zoom_scale);
	gl_Position = vec4(pos.xy, 0.0, 1.0);
}
