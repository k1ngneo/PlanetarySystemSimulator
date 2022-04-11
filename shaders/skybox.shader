#vertex_shader
#version 330 core

layout (location = 0) in vec3 inPos;

uniform mat4 _viewMat;
uniform mat4 _projMat;

out vec3 fragUV;

void main() {
	gl_Position = _projMat * _viewMat * vec4(inPos, 1.0);
	fragUV = inPos;
}

#fragment_shader
#version 330 core

in vec3 fragUV;

uniform samplerCube _cubeMap;

out vec4 outColor;

void main() {
	vec3 background = texture(_cubeMap, fragUV).rgb;
	outColor = vec4(background, 1.0);
}