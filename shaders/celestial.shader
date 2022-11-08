#vertex_shader
#version 430 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNorm;
layout (location = 3) in vec3 inTan;

uniform mat4 _modelMat;
uniform mat4 _viewMat;

out vec3 csPos;
out vec3 csVPos;

void main() {
    csPos = inPos;
    csVPos = vec3(_viewMat * _modelMat * vec4(inPos, 1.0));
}

#control_shader
#version 430 core

layout (vertices = 3) out;

in vec3 csPos[];
in vec3 csVPos[];

out vec3 esPos[];

uniform mat4 _modelMat;
uniform mat4 _viewMat;

void main() {
    esPos[gl_InvocationID] = csPos[gl_InvocationID];

    float tessellationLevel, camDist;
    const float tessFactor = 4.0;

    // calculating outer tessellation levels
    vec3 edgePos = 0.5 * (csVPos[gl_InvocationID] + csVPos[(gl_InvocationID+1)%3]);
    camDist = distance(vec3(0.0), edgePos);
    tessellationLevel = 0.0001;
    if(camDist >= 0.0001) {
        tessellationLevel = tessFactor / (camDist*camDist);
    }

    gl_TessLevelOuter[gl_InvocationID] = tessellationLevel;

    // calculating inner tessellation level
    if(gl_InvocationID == 0) {
        tessellationLevel = 0.0001;
        camDist = distance(vec3(0.0), 0.333 * (csVPos[0] + csVPos[1] + csVPos[2]));
        if(camDist >= 0.0001) {
            tessellationLevel = tessFactor / (camDist*camDist);
        }
        gl_TessLevelInner[0] = tessellationLevel;
    }
}

#evaluation_shader
#version 430 core

layout (triangles, equal_spacing, ccw) in;

in vec3 esPos[];

out vec2 gsUV;

void main() {
    // vertex position
    vec3 vertexPos = vec3(0.0);
    vertexPos += esPos[0] * gl_TessCoord.x;
    vertexPos += esPos[1] * gl_TessCoord.y;
    vertexPos += esPos[2] * gl_TessCoord.z;
    vertexPos = normalize(vertexPos);

    gl_Position = vec4(vertexPos, 1.0);

    // uv coordinates
    {
        float lng, lat;
        const float INFINITY = uintBitsToFloat(0x7F800000);
        const float PI = 3.14159265359;

        // sina = y / radius
        // a = arcsin(y / radius)
        lat = -asin(vertexPos.y);

        float px, pz;
        if(vertexPos.x > 0.0)
            px = vertexPos.x;
        else {
            px = -vertexPos.x;
        }
        
        if(vertexPos.z > 0.0)
            pz = vertexPos.z;
        else {
            pz = -vertexPos.z;
        }

        if(px == 0.0 || px == -0.0)
            lng = atan(INFINITY);
        else
            lng = atan(pz / px);
        
        if(vertexPos.z >= 0.0 && vertexPos.x >= 0.0) {
            lng = 0.5 * PI - lng;
        }
        else if(vertexPos.z <= 0.0 && vertexPos.x >= 0.0) {
            lng += 0.5 * PI;
        }
        else if(vertexPos.z <= 0.0 && vertexPos.x <= 0.0) {
            lng = 0.5 * PI - lng;
            lng += PI;
        }
        else if(vertexPos.z >= 0.0 && vertexPos.x < 0.0) {
            lng += 1.5 * PI;
        }

        gsUV.x = lng / (2.0 * PI);
        gsUV.y = (lat + 0.5 * PI) / PI;
    }
}

#geometry_shader
#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

struct Light {
    vec3 tanPos;

    vec3 amb;
    vec3 diff;
    vec3 spec;
    vec3 att;
};

in vec2 gsUV[3];

uniform mat4 _modelMat;
uniform mat4 _viewMat;
uniform mat4 _projMat;

uniform vec3 _viewPos;
uniform Light _light;

out FS_Data {
    vec2 uv;
    vec3 tanPos;
    vec3 tanCamPos;

    Light light;
} fsData;

void main() {

    vec3 worldPos[3], worldNormal[3];
    for(int vertexInd = 0; vertexInd < 3; vertexInd += 1) {
        worldPos[vertexInd] = vec3(_modelMat * vec4(gl_in[vertexInd].gl_Position));
        worldNormal[vertexInd] = vec3(_modelMat * vec4(gl_in[vertexInd].gl_Position.xyz, 0.0));
    }

    // fixing uv coordinates
    vec2 fixedUV[3];
    {
        fixedUV[0] = gsUV[0];
        fixedUV[1] = gsUV[1];
        fixedUV[2] = gsUV[2];

        // fixing highest values of U coordinates
        // doesn't seem to work when there are verticies on the right edge of the texture
        // (uv = [1.0, v])
        vec3 uvA = vec3(gsUV[0], 0.0);
        vec3 uvB = vec3(gsUV[1], 0.0);
        vec3 uvC = vec3(gsUV[2], 0.0);

        vec3 uvNormal = cross(uvB - uvA, uvC - uvA);
        if(uvNormal.z < 0.0) {
            if(uvA.x < 0.4) {
                fixedUV[0].x += 1.0;
            }
            if(uvB.x < 0.4) {
                fixedUV[1].x += 1.0;
            }
            if(uvC.x < 0.4) {
                fixedUV[2].x += 1.0;
            }
        }

        // fixing uvs at the poles
        // doesn't seem to work at all
        if(uvA.y == 0.0 || uvA.y == 1.0) {
            fixedUV[0] = vec2(0.5*(uvB.x + uvC.x), uvA.y);
        }
        if(uvB.y == 0.0 || uvB.y == 1.0) {
            fixedUV[1] = vec2(0.5*(uvA.x + uvC.x), uvB.y);
        }
        if(uvC.y == 0.0 || uvC.y == 1.0) {
            fixedUV[2] = vec2(0.5*(uvA.x + uvB.x), uvC.y);
        }
    }

    // calculating TBN matrix
    mat3 TBNMat[3];
    {
        // calculating tangent and bitangent
        vec3 worldTan[3], worldBTan[3];
        {
            vec3 dPos1 = worldPos[1] - worldPos[0];
            vec3 dPos2 = worldPos[2] - worldPos[0];
            vec2 dUV1 = fixedUV[1] - fixedUV[0];
            vec2 dUV2 = fixedUV[2] - fixedUV[0];

            float f = 1.0 / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);

            vec3 tan;
            tan.x = f * (dUV2.y * dPos1.x - dUV1.y * dPos2.x);
			tan.y = f * (dUV2.y * dPos1.y - dUV1.y * dPos2.y);
			tan.z = f * (dUV2.y * dPos1.z - dUV1.y * dPos2.z);
            tan = normalize(tan);

            // making the tangent vector orthogonal to the normal vector
            for(int vertexInd = 0; vertexInd < 3; vertexInd += 1) {
                worldTan[vertexInd] = normalize(tan - dot(tan, worldNormal[vertexInd]) * worldNormal[vertexInd]);
                worldBTan[vertexInd] = normalize(cross(worldNormal[vertexInd], worldTan[vertexInd]));
            }
        }

        for(int vertexInd = 0; vertexInd < 3; vertexInd += 1) {
            TBNMat[vertexInd] = transpose(mat3(worldTan[vertexInd], worldBTan[vertexInd], worldNormal[vertexInd]));
        }
    }

    for(int vertexInd = 0; vertexInd < 3; vertexInd += 1) {
        gl_Position = _projMat * _viewMat * _modelMat * gl_in[vertexInd].gl_Position;

        fsData.uv = fixedUV[vertexInd];
        fsData.tanPos = TBNMat[vertexInd] * worldPos[vertexInd];
        fsData.tanCamPos = TBNMat[vertexInd] * _viewPos;

        fsData.light = _light;
        fsData.light.tanPos = TBNMat[vertexInd] * _light.tanPos;

        EmitVertex();
    }

    EndPrimitive();
}

#fragment_shader
#version 430 core

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
} fsData;

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

    vec3 lightDir = normalize(light.tanPos - fsData.tanPos);
    vec3 viewDir = normalize(fsData.tanCamPos - fsData.tanPos);

    float diff = max(dot(mater.norm, -lightDir), 0.0);
    vec3 diffuseLight = (mater.diff * diff) * light.diff;

    vec3 reflectDir = reflect(lightDir, mater.norm);

    float spec = pow(max(dot(-viewDir, reflectDir), 0.0), mater.shine);
    vec3 specularLight = (mater.spec * spec) * light.spec;

    float dist = length(light.tanPos - fsData.tanPos);
    float attenuation = 1.0 / (light.att.x + light.att.y * dist
        + light.att.y * (dist*dist));

    lightSum = /*ambientLight +*/ diffuseLight /*+ specularLight*/;
    //lightSum *= attenuation;

    return lightSum;
}

void main() {
    Material mater;
    mater.amb = vec3(0.0);
    mater.diff = texture2D(_diffTex, fsData.uv).rgb;
    mater.spec = texture2D(_specTex, fsData.uv).rgb;
    mater.shine = 32.0;

    float waveSize = 20.0;
    vec3 wave1 = texture2D(_otherTex1, waveSize*fsData.uv + 0.03*vec2(_time, 0.0)).rgb;
    vec3 wave2 = texture2D(_otherTex2, waveSize*fsData.uv + 0.03*vec2(0.0, _time)).rgb;
    vec3 waves_normal = normalize(2.0 * (0.5*(wave1+wave2)) - 1.0);
    vec3 land_normal = normalize(2.0 * texture2D(_normTex, fsData.uv).rgb - 1.0);

    mater.norm = mix(land_normal, waves_normal, mater.spec.r);
    //mater.norm = mix(mater.norm, vec3(0.0, 0.0, 1.0), 0.8);

    mater.night = texture2D(_nightTex, fsData.uv).rgb;

    vec3 pointLights = calcPointLight(fsData.light, mater);

    float darkness = 0.3334 * (pointLights.r + pointLights.g + pointLights.b);
    vec3 nightLights = smoothstep(0.08, 0.0, darkness) * mater.night;


    outColor = vec4(pointLights + nightLights, 1.0);
    outColor = vec4(mater.norm, 1.0);
    outColor = vec4(fsData.tanCamPos, 1.0);
    outHDRColor = vec4(0.0, 0.0, 0.0, 1.0);
}