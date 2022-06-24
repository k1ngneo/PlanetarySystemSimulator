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

out vec4 outColor;

void main() {
	outColor = texture2D(_texture, fragUV);
	float average = 0.33*(outColor.r + outColor.g + outColor.b);
	outColor = vec4(average, average, average, 1.0);
}