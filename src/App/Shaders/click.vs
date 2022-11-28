#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main() {
    vec3 modifiedPos = aPos;
    modifiedPos.y += sin(time * (modifiedPos.x + modifiedPos.z) * 2 + time * 30) * 0.4;

	gl_Position = projection * view * model * vec4(modifiedPos, 1.0);
}
