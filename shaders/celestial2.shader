#vertex_shader
#version 430 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNorm;
layout (location = 3) in vec3 inTan;

uniform mat4 _modelMat;
uniform mat4 _viewMat;
uniform mat4 _projMat;

out vec3 csPos;
out vec3 origin;
out vec3 csVPos;

void main() {
    csPos = vec3(_modelMat * vec4(inPos, 1.0));
    csVPos = vec3(_viewMat * vec4(csPos, 1.0));
    origin = vec3(_viewMat * _modelMat * vec4(vec3(0.0), 1.0));
}

#control_shader
#version 430 core

layout (vertices = 3) out;

in vec3 csPos[];
out vec3 esPos[];

uniform mat4 _projMat;
uniform float _TL;

void main() {
    esPos[gl_InvocationID] = csMPos[gl_InvocationID];

    float tessellationLevel;
    float camDist = distance(vec3(0.0), csVPos[gl_InvocationID]);
    if(camDist != 0.0) {
        tessellationLevel = 9.0 / camDist;
    }
    else {
        tessellationLevel = 1.0;
    }

    gl_TessLevelOuter[0] = tessellationLevel;
    gl_TessLevelOuter[1] = tessellationLevel;
    gl_TessLevelOuter[2] = tessellationLevel;
    gl_TessLevelInner[0] = tessellationLevel;
}

#evaluation_shader
#version 430 core

layout (triangles, fractional_even_spacing, ccw) in;

uniform mat4 _modelMat;
uniform mat4 _viewMat;
uniform mat4 _projMat;

in vec3 esPos[];

void main() {
    vec3 vertexPos = esPos[0] * vec3(gl_TessCoord.x) + esPos[1] * vec3(gl_TessCoord.y) + esPos[2] * vec3(gl_TessCoord.z);
    vertexPos = normalize(vertexPos);
    gl_Position = _projMat * _viewMat * _modelMat * vec4(vertexPos, 1.0);


}

/*
struct Light {
    vec3 pos;

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
*/

#fragment_shader
#version 430 core

in vec2 fragUV;
in vec3 fragNorm;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outHDRColor;

void main() {
    outColor = vec4(1.0, 0.1, 0.1, 1.0);
    outHDRColor = vec4(0.0, 0.0, 0.0, 1.0);
}

/*
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

uniform mat4 _viewMat;

uniform sampler2D _diffTex;
uniform sampler2D _specTex;
uniform sampler2D _normTex;
uniform sampler2D _nightTex;
uniform sampler2D _otherTex1;
uniform sampler2D _otherTex2;

uniform float _time;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outHDRColor;

vec3 calcPointLight(Light light, Material mater) {
    vec3 lightSum;

    vec3 ambientLight = mater.amb * light.amb;

    float diff = max(dot(mater.norm, -light.dir), 0.0);
    vec3 diffuseLight = (mater.diff * diff) * light.diff;

    vec3 reflectDir = reflect(light.dir, mater.norm);

    float spec = pow(max(dot(-fragData.viewDir, reflectDir), 0.0), mater.shine);
    vec3 specularLight = (mater.spec * spec) * light.spec;

    float dist = length(light.pos - fragData.pos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * dist
        + light.attenuation.y * (dist*dist));

    lightSum = ambientLight + diffuseLight + specularLight;
    lightSum *= attenuation;

    return lightSum;
}

void main() {
    Material mater;
    mater.amb = vec3(0.0);
    mater.diff = texture2D(_diffTex, fragData.uv).rgb;
    mater.spec = texture2D(_specTex, fragData.uv).rgb;
    mater.shine = 32.0;

    float waveSize = 20.0;
    vec3 wave1 = texture2D(_otherTex1, waveSize*fragData.uv + 0.03*vec2(_time, 0.0)).rgb;
    vec3 wave2 = texture2D(_otherTex2, waveSize*fragData.uv + 0.03*vec2(0.0, _time)).rgb;
    vec3 waves_normal = normalize(2.0 * (0.5*(wave1+wave2)) - 1.0);
    vec3 land_normal = normalize(2.0 * texture2D(_normTex, fragData.uv).rgb - 1.0);

    mater.norm = mix(land_normal, waves_normal, mater.spec.r);
    mater.norm = mix(mater.norm, vec3(0.0, 0.0, 1.0), 0.8);

    mater.night = texture2D(_nightTex, fragData.uv).rgb;

    vec3 pointLights = calcPointLight(fragData.light1, mater);

    float darkness = 0.3334 * (pointLights.r + pointLights.g + pointLights.b);
    vec3 nightLights = smoothstep(0.08, 0.0, darkness) * mater.night;

    outColor = vec4(pointLights + nightLights, 1.0);
    outHDRColor = vec4(0.0, 0.0, 0.0, 1.0);
}
*/