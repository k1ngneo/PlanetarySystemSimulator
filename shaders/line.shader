#vertex_shader
#version 430 core

layout (location=0) in vec3 pos;

uniform mat4 _projMat;
uniform mat4 _viewMat;

void main() {
	gl_Position = _projMat * _viewMat * vec4(pos, 1.0);
}

#fragment_shader
#version 430 core

layout (location=0) out vec4 outColor;

void main() {
	outColor = vec4(0.0, 1.0, 0.0, 1.0);
}