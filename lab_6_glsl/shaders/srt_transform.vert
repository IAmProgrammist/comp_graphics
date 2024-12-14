#version 330 core
layout(location = 0) in vec3 position;
uniform mat4 general_matrix;
void main() {
    gl_Position = general_matrix * vec4(position, 1.0);
}