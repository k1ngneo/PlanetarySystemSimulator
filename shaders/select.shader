#vertex_shader
#version 330 core

layout (location = 0) in vec3 inPos;

uniform mat4 _projMat;
uniform mat4 _viewMat;
uniform mat4 _modelMat;

out struct FragData {
    vec3 pos;
} fragData;

void main() {
    fragData.pos = vec3(_modelMat * vec4(1.02 * inPos, 1.0));
    gl_Position = _projMat * _viewMat * vec4(fragData.pos, 1.0);
}

#fragment_shader
#version 330 core

in struct FragData {
    vec3 pos;
} fragData;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0.9, 0.1, 0.9, 1.0);
}
