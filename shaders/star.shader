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


#if __VERSION__ > 120
#define TEXTURE2D(tID, uv) texture(tID, uv)
#else
#define TEXTURE2D(tID, uv) texture2D(tID, uv)
#endif

in vec2 fragUV;

uniform sampler2D _diffTex;
uniform vec3 _diffColor;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outHDRColor;

void main() {
	vec3 texCol = TEXTURE2D(_diffTex, fragUV).rgb;
	outColor = vec4(_diffColor * texCol, 1.0);
	outHDRColor = vec4(15.0 * outColor.rgb, 1.0);
}
