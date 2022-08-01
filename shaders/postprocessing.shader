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

in vec2 fragUV;

uniform sampler2D _texture;
uniform sampler2D _bloomTex;

layout (location = 0) out vec4 outColor;

void main() {
	vec3 mainImage = texture2D(_texture, fragUV).rgb;
	vec3 bloom = texture2D(_bloomTex, fragUV).rgb;

	vec2 uvOffset = textureSize(_texture, 0);
	vec2 invScrSize = 1.0 / uvOffset;
	vec2 offsets[8] = vec2[] (
		vec2(-uvOffset.x,  uvOffset.y), // (0) left up
		vec2(        0.0,  uvOffset.y), // (1) up
		vec2( uvOffset.x,  uvOffset.y), // (2) right up
		vec2(-uvOffset.x,         0.0), // (3) left
		vec2( uvOffset.x,         0.0), // (4) right
		vec2(-uvOffset.x, -uvOffset.y), // (5) left down
		vec2(        0.0, -uvOffset.y), // (6) down
		vec2( uvOffset.x, -uvOffset.y)  // (7) right down
	);

	outColor = vec4(mainImage, 1.0);
	//outColor = vec4(bloom, 1.0);
}