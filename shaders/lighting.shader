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
uniform Light _light2;

out struct FragData {
    vec3 pos;
    vec2 uv;
    vec3 posViewSp;
    vec3 posTan;

    vec3 viewDir;

    Light light1;
} fragData;

void main() {
    fragData.pos = vec3(_modelMat * vec4(inPos, 1.0));
    fragData.posViewSp = vec3(_viewMat * vec4(fragData.pos, 1.0));
    gl_Position = _projMat * vec4(fragData.posViewSp, 1.0);
    fragData.uv = inUV;

    // calculating TBN matrix
    vec3 Norm = normalize(vec3(_modelMat * vec4(inNorm, 0.0)));
    vec3 Tan = normalize(vec3(_modelMat * vec4(inTan, 0.0)));
    Tan = normalize(Tan - dot(Tan, Norm) * Norm);
    vec3 BTan = normalize(cross(Norm, Tan));
    mat3 TBN = transpose(mat3(Tan, BTan, Norm));

    fragData.posTan = TBN * fragData.pos;
    fragData.viewDir = normalize(fragData.posTan - TBN * _viewPos);

    fragData.light1 = _light1;
    fragData.light1.pos = TBN * _light1.pos;
    fragData.light1.dir = normalize(fragData.posTan - fragData.light1.pos);
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
    vec2 uv;
    vec3 posViewSp;
    vec3 posTan;

    vec3 viewDir;

    Light light1;
} fragData;

struct Material {
    vec3 amb;
    vec3 diff;
    vec3 spec;
    float shine;

    vec3 norm;
    vec3 night;
};

uniform Material _surfM;
uniform mat4 _viewMat;

uniform sampler2D _diffTex;
uniform sampler2D _specTex;
uniform sampler2D _normTex;
uniform sampler2D _nightTex;

uniform float _time;

out vec4 outColor;

vec3 calcPointLight(Light light, Material mater) {
    vec3 lightSum;

    vec3 ambientLight = mater.amb * light.amb;

    float diff = max(dot(mater.norm, -light.dir), 0.0);
    vec3 diffuseLight = (mater.diff * diff) * light.diff;

    vec3 reflectDir = reflect(light.dir, mater.norm);

    float spec = pow(max(dot(-fragData.viewDir, reflectDir), 0.0), mater.shine);
    vec3 specularLight = (/*mater.spec */ spec) * light.spec;

    float dist = length(light.pos - fragData.pos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * dist
        + light.attenuation.y * (dist*dist));

    lightSum = ambientLight + diffuseLight + specularLight;
    lightSum *= attenuation;

    return lightSum;
}

void main() {
    Material mater;
    mater.amb = _surfM.amb;
    mater.diff = _surfM.diff * texture2D(_diffTex, fragData.uv).rgb;
    mater.spec = _surfM.spec * texture2D(_specTex, fragData.uv).rgb;
    mater.shine = _surfM.shine;

    mater.norm = normalize(texture2D(_normTex, fragData.uv).rgb * 2.0 - 1.0);

    mater.night = texture2D(_nightTex, fragData.uv).rgb;

    vec3 pointLights = calcPointLight(fragData.light1, mater);

    float darkness = 0.3334 * (pointLights.r + pointLights.g + pointLights.b);
    vec3 nightLights = smoothstep(0.1, 0.0, darkness) * mater.night;

    outColor = vec4(pointLights + nightLights, 1.0);
}
