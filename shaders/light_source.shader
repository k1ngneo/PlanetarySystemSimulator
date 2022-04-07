#vertex_shader
#version 330 core

layout (location = 0) in vec3 inPos;

uniform mat4 _projMat;
uniform mat4 _viewMat;
uniform mat4 _modelMat;

void main(){
    gl_Position = _projMat * _viewMat * _modelMat * vec4(inPos, 1.0);
}

#fragment_shader
#version 330 core

uniform vec3 _lightCol;

out vec4 outColor;

void main(){
    outColor = vec4(_lightCol, 1.0);
}
