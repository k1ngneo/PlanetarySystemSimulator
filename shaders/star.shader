#vertex_shader
#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

uniform mat4 _modelMat;
uniform mat4 _viewMat;
uniform mat4 _projMat;

out vec2 fragUV;

void main() {
	gl_Position = _projMat * _viewMat * _modelMat * vec4(inPos, 1.0);
	fragUV = inUV;
}

#fragment_shader
#version 330 core

in vec2 fragUV;

uniform sampler2D _diffTex;

out vec4 outColor;

void main() {
	vec3 texCol = texture2D(_diffTex, fragUV).rgb;
	outColor = vec4(texCol, 1.0);
}