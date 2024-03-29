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


#if __VERSION__ > 120
#define TEXTURE2D(tID, uv) texture(tID, uv)
#else
#define TEXTURE2D(tID, uv) texture2D(tID, uv)
#endif

in vec3 fragUV;

uniform samplerCube _cubeMap;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outHDRColor;

void main() {
	vec3 background = TEXTURE2D(_cubeMap, fragUV).rgb;
	outColor = vec4(background, 1.0);
	outHDRColor = vec4(vec3(0.0), 1.0);
}
