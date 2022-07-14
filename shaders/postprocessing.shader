#vertex_shader
#version 330 core

layout (location=0) in vec2 inPos;
layout (location=1) in vec2 inUV;

out vec2 fragUV;

void main() {
	gl_Position = vec4(inPos, 0.0, 1.0);
	fragUV = inUV;
}

#fragment_shader
#version 330 core

in vec2 fragUV;

uniform sampler2D _texture;
uniform sampler2D _bloomTex;

layout (location = 0) out vec4 outColor;

void main() {
	vec3 mainImage = texture2D(_texture, fragUV).rgb;
	vec3 bloom = texture2D(_bloomTex, fragUV).rgb;

	vec2 uvOffset = textureSize(_texture, 0);
	vec2 offsets[9] = vec2[] (
		vec2(-uvOffset.x,  uvOffset.y),
		vec2(        0.0,  uvOffset.y),
		vec2( uvOffset.x,  uvOffset.y),
		vec2(-uvOffset.x,         0.0),
		vec2(        0.0,         0.0),
		vec2( uvOffset.x,         0.0),
		vec2(-uvOffset.x, -uvOffset.y),
		vec2(        0.0, -uvOffset.y),
		vec2( uvOffset.x, -uvOffset.y)
	);

	float edgesKernel[9] = float[] (
		1.0, 1.0, 1.0,
		1.0, -7.0, 1.0,
		1.0, 1.0, 1.0
	);

	// normalizing kernel
	float sum = 0.0;
	for(int i = 0; i < 9; ++i) {
		sum += edgesKernel[i];
	}
	for(int i = 0; i < 9; ++i) {
		edgesKernel[i] /= sum;
	}

	vec3 smoothedImage = vec3(0.0, 0.0, 0.0);
	for(int i = 0; i < 9; ++i) {
		smoothedImage += texture2D(_texture, fragUV + offsets[i]).rgb * edgesKernel[i];
	}

	outColor = vec4(mainImage + bloom, 1.0);
}