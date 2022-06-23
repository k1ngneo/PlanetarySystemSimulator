#vertex_shader
#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNorm;
layout (location = 3) in vec3 inTan;

struct Light {
    vec3 pos;
    vec3 dir;

    vec3 amb;
    vec3 diff;
    vec3 spec;

    vec3 attenuation;
};

uniform mat4 _projMat;
uniform mat4 _viewMat;
uniform mat4 _modelMat;

uniform vec3 _viewPos;

uniform Light _light1;

out struct FragData {
    vec3 pos;
	vec3 norm;
    vec3 viewDir;
    Light light1;
} fragData;

void main() {
    fragData.pos = vec3(_modelMat * vec4(1.02 * inPos, 1.0));
    vec3 posViewSp = vec3(_viewMat * vec4(fragData.pos, 1.0));
    gl_Position = _projMat * vec4(posViewSp, 1.0);

    // calculating TBN matrix
    fragData.norm = normalize(vec3(_modelMat * vec4(-inNorm, 0.0)));
	fragData.norm = normalize(vec3(_modelMat * vec4(inPos, 0.0)));
    fragData.viewDir = normalize(posViewSp - _viewPos);

    fragData.light1 = _light1;
    fragData.light1.dir = normalize(fragData.pos - fragData.light1.pos);
}

#fragment_shader
#version 330 core

struct Light {
    vec3 pos;
    vec3 dir;

    vec3 amb;
    vec3 diff;
    vec3 spec;

    vec3 attenuation;
};

in struct FragData {
    vec3 pos;
	vec3 norm;
    vec3 viewDir;
    Light light1;
} fragData;

out vec4 outColor;

float calcPointLight(Light light) {
    vec3 lightSum;

    float diff = max(dot(fragData.norm, -light.dir), 0.0);
    vec3 diffuseLight = diff * light.diff;

    vec3 reflectDir = reflect(light.dir, fragData.norm);

    float spec = max(dot(-fragData.viewDir, reflectDir), 0.0);
    vec3 specularLight = spec * light.spec;

    float dist = length(light.pos - fragData.pos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * dist
        + light.attenuation.y * (dist*dist));

    lightSum = diffuseLight + specularLight;
    lightSum *= attenuation;

	float value = diff * attenuation;

    return value;
}

void main() {
	float colorAlpha = calcPointLight(fragData.light1);
    outColor = vec4(0.1, 0.1, 0.9, colorAlpha);
}
