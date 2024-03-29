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

uniform sampler2D _texture;

uniform bool _horizontal;
uniform float _weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

layout (location = 0) out vec4 outColor;

void main() {
	vec2 texelSize = 1.0 / textureSize(_texture, 0);
	vec3 result = texture2D(_texture, fragUV).rgb * _weight[0];

	vec2 offsetPos;

	if(_horizontal) {
		for(int i = 1; i < 5; ++i) {
			offsetPos = vec2(fragUV.x + texelSize.x * i, fragUV.y);
			if(offsetPos.x >= 1.0)
				offsetPos.x = 0.999;
			result += texture2D(_texture, offsetPos).rgb * _weight[i];
			
			offsetPos = vec2(fragUV.x - texelSize.x * i, fragUV.y);
			if(offsetPos.x <= 0.0)
				offsetPos.x = 0.001;
			result += texture2D(_texture, offsetPos).rgb * _weight[i];
		}
	}
	else {
		for(int i = 1; i < 5; ++i) {
			offsetPos = vec2(fragUV.x, fragUV.y + texelSize.y * i);
			if(offsetPos.y >= 1.0)
				offsetPos.y = 0.999;
			result += texture2D(_texture, offsetPos).rgb * _weight[i];
			
			offsetPos = vec2(fragUV.x, fragUV.y - texelSize.y * i);
			if(offsetPos.y <= 0.0)
				offsetPos.y = 0.001;
			result += texture2D(_texture, offsetPos).rgb * _weight[i];
		}
	}

	outColor = vec4(result, 1.0);
}