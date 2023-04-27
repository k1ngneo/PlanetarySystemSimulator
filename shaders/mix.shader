#vertex_shader
#version 330 core

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;

out vec2 fragUV;

void main() {
	gl_Position = vec4(inPos, 0.0, 1.0);
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

uniform sampler2D _texture1;
uniform sampler2D _texture2;

layout (location = 0) out vec4 outColor;

void main() {
	vec3 mixedColors = 0.5 * (texture2D(_texture1, fragUV).rgb + texture2D(_texture2, fragUV).rgb);
	outColor = vec4(mixedColors, 1.0);
}