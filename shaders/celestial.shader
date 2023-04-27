#vertex_shader
#version 430 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;

uniform mat4 _modelMat;
uniform mat4 _viewMat;

out CS_Data {
    vec3 mPos;
    vec3 vPos;
    vec2 uv;
    mat3 TBNMat;

    vec3 wNormal;
    vec3 wTangent;
} csData;

void main() {
    csData.mPos = inPos;
    csData.vPos = vec3(_viewMat * _modelMat * vec4(inPos, 1.0));
    csData.uv = inUV;

    csData.wNormal = normalize(vec3(_modelMat * vec4(inNormal, 0.0)));
    csData.wTangent = vec3(_modelMat * vec4(inTangent, 0.0));

    csData.wTangent = normalize(csData.wTangent - dot(csData.wTangent, csData.wNormal) * csData.wNormal);
    vec3 wBiTan = normalize(cross(csData.wNormal, csData.wTangent));
    csData.TBNMat = transpose(mat3(csData.wTangent, wBiTan, csData.wNormal));
}

#control_shader
#version 430 core

layout (vertices = 3) out;

in CS_Data {
    vec3 mPos;
    vec3 vPos;
    vec2 uv;
    mat3 TBNMat;

    vec3 wNormal;
    vec3 wTangent;
} csData[];

out ES_Data {
    vec3 pos;
    vec2 uv;
    mat3 TBNMat;

    vec3 wNormal;
    vec3 wTangent;
} esData[];

uniform mat4 _modelMat;
uniform mat4 _viewMat;

void main() {
    esData[gl_InvocationID].pos = csData[gl_InvocationID].mPos;
    esData[gl_InvocationID].uv = csData[gl_InvocationID].uv;
    esData[gl_InvocationID].TBNMat = csData[gl_InvocationID].TBNMat;

    esData[gl_InvocationID].wNormal = csData[gl_InvocationID].wNormal;
    esData[gl_InvocationID].wTangent = csData[gl_InvocationID].wTangent;

    if(gl_InvocationID == 0) {
        float camDist;
        const float tessFactor = 4.0;

        // calculating outer tessellation levels
        for(int vertexInd = 0; vertexInd < 3; vertexInd += 1) {
            vec3 edgePos = 0.5 * (csData[vertexInd].vPos + csData[(vertexInd+1)%3].vPos);
            camDist = -edgePos.z;

            gl_TessLevelOuter[vertexInd] = tessFactor / (camDist);
        }

        // calculating inner tessellation level
        vec3 triangleCenter = 0.33333 * (csData[0].vPos + csData[1].vPos + csData[2].vPos);
        camDist = -triangleCenter.z;
        gl_TessLevelInner[0] = tessFactor / (camDist);
    }
}

#evaluation_shader
#version 430 core


#if __VERSION__ > 120
#define TEXTURE2D(tID, uv) texture(tID, uv)
#else
#define TEXTURE2D(tID, uv) texture2D(tID, uv)
#endif

layout (triangles, fractional_odd_spacing, ccw) in;

in ES_Data {
    vec3 pos;
    vec2 uv;
    mat3 TBNMat;

    vec3 wNormal;
    vec3 wTangent;
} esData[];

struct Light {
    vec3 tanPos;

    vec3 amb;
    vec3 diff;
    vec3 spec;
    vec3 att;
};

uniform Light _light;
uniform vec3 _viewPos;

uniform mat4 _projMat;
uniform mat4 _viewMat;
uniform mat4 _modelMat;

uniform sampler2D _specHeightTex;

out FS_Data {
    vec2 uv;
    vec3 tanPos;
    vec3 tanCamPos;

    Light light;

    vec3 wNormal;
    vec3 wTangent;
} fsData;

void main() {
    // uv coordinates
    fsData.uv = vec2(0.0);
    fsData.uv += esData[0].uv * gl_TessCoord.y;
    fsData.uv += esData[1].uv * gl_TessCoord.z;
    fsData.uv += esData[2].uv * gl_TessCoord.x;

    // vertex position
    vec3 vertexPos = vec3(0.0);
    vertexPos += esData[0].pos * gl_TessCoord.y;
    vertexPos += esData[1].pos * gl_TessCoord.z;
    vertexPos += esData[2].pos * gl_TessCoord.x;
    vertexPos = normalize(vertexPos) * (1.0 + 0.03 * TEXTURE2D(_specHeightTex, fsData.uv).g);
    vertexPos = vec3(_modelMat * vec4(vertexPos, 1.0));

    gl_Position = _projMat * _viewMat * vec4(vertexPos, 1.0);

    fsData.wNormal = esData[0].wNormal * gl_TessCoord.y;
    fsData.wNormal += esData[1].wNormal * gl_TessCoord.z;
    fsData.wNormal += esData[2].wNormal * gl_TessCoord.x;

    fsData.wTangent = esData[0].wTangent * gl_TessCoord.y;
    fsData.wTangent += esData[1].wTangent * gl_TessCoord.z;
    fsData.wTangent += esData[2].wTangent * gl_TessCoord.x;

    // tangent-space matrix
    mat3 TBNMat = mat3(0.0);
    TBNMat += esData[0].TBNMat * gl_TessCoord.y;
    TBNMat += esData[1].TBNMat * gl_TessCoord.z;
    TBNMat += esData[2].TBNMat * gl_TessCoord.x;

    fsData.tanPos = TBNMat * vertexPos;
    fsData.tanCamPos = TBNMat * _viewPos;
    fsData.light = _light;
    fsData.light.tanPos = TBNMat * _light.tanPos;
}

#fragment_shader
#version 430 core


#if __VERSION__ > 120
#define TEXTURE2D(tID, uv) texture(tID, uv)
#else
#define TEXTURE2D(tID, uv) texture2D(tID, uv)
#endif

struct Light {
    vec3 tanPos;

    vec3 amb;
    vec3 diff;
    vec3 spec;
    vec3 att;
};

struct Material {
    vec3 amb;
    vec3 diff;
    vec3 spec;
    float shine;

    vec3 norm;
    vec3 night;
};

in FS_Data {
    vec2 uv;
    vec3 tanPos;
    vec3 tanCamPos;

    Light light;

    vec3 wNormal;
    vec3 wTangent;
} fsData;

uniform sampler2D _diffTex;
uniform sampler2D _specHeightTex;
uniform sampler2D _nightTex;
uniform sampler2D _otherTex1;
uniform sampler2D _otherTex2;

uniform float _time;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outHDRColor;

vec3 calcPointLight(Light light, Material mater) {
    vec3 lightSum;

    vec3 ambientLight = mater.amb * light.amb;

    vec3 lightDir = normalize(light.tanPos - fsData.tanPos);
    vec3 viewDir = normalize(fsData.tanCamPos - fsData.tanPos);

    float diff = max(dot(mater.norm, lightDir), 0.0);
    vec3 diffuseLight = (mater.diff * diff) * light.diff;

    vec3 reflectDir = reflect(lightDir, mater.norm);

    float spec = pow(max(dot(-viewDir, reflectDir), 0.0), mater.shine);
    vec3 specularLight = (mater.spec * spec) * light.spec;

    float dist = length(light.tanPos - fsData.tanPos);
    float attenuation = 1.0 / (light.att.x + light.att.y * dist
        + light.att.y * (dist*dist));

    lightSum = ambientLight + diffuseLight + specularLight;
    lightSum *= attenuation;

    return lightSum;
}

void main() {
    Material mater;
    mater.amb = vec3(0.0);
    mater.diff = TEXTURE2D(_diffTex, fsData.uv).rgb;
    mater.spec = TEXTURE2D(_specHeightTex, fsData.uv).rrr;
    mater.shine = 16.0;

    float waveSize = 20.0;
    vec3 wave1 = TEXTURE2D(_otherTex1, waveSize*fsData.uv + 0.03*vec2(_time, 0.0)).rgb;
    vec3 wave2 = TEXTURE2D(_otherTex2, waveSize*fsData.uv + 0.03*vec2(0.0, _time)).rgb;
    vec3 waves_normal = normalize(2.0 * (0.5*(wave1+wave2)) - 1.0);
    vec3 land_normal = vec3(0.0, 0.0, 1.0);

    mater.norm = mix(land_normal, waves_normal, mater.spec.r);
    mater.norm = mix(mater.norm, vec3(0.0, 0.0, 1.0), 0.8);

    mater.night = TEXTURE2D(_nightTex, fsData.uv).rgb;

    vec3 pointLights = calcPointLight(fsData.light, mater);

    float darkness = 0.3334 * (pointLights.r + pointLights.g + pointLights.b);
    vec3 nightLights = smoothstep(0.08, 0.0, darkness) * mater.night;


    outColor = vec4(pointLights + nightLights, 1.0);
    outHDRColor = vec4(0.0, 0.0, 0.0, 1.0);
}
