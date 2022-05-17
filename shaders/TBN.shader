#vertex_shader
#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNorm;
layout (location = 3) in vec3 inTan;

uniform mat4 _projMat;
uniform mat4 _viewMat;
uniform mat4 _modelMat;

out GS_IN {
    vec3 norm;
    vec3 tan;
    vec3 bTan;
} gs_in;

void main() {
    mat4 locToProjMat = _projMat * _viewMat * _modelMat;
    gl_Position = locToProjMat * vec4(inPos, 1.0);

    vec3 norm = normalize(vec3(locToProjMat * vec4(inNorm, 0.0)));
    vec3 tan = normalize(vec3(locToProjMat * vec4(inTan, 0.0)));
    tan = normalize(tan - dot(tan, norm) * norm);
    vec3 bTan = normalize(cross(norm, tan));

    gs_in.norm = norm;
    gs_in.tan = tan;
    gs_in.bTan = bTan;
}

#geometry_shader
#version 330 core

layout (points) in;
layout (line_strip, max_vertices = 6) out;

in GS_IN {
    vec3 norm;
    vec3 tan;
    vec3 bTan;
} gs_in[];

out vec3 fragColor;

void main() {
    fragColor = vec3(0.3, 0.3, 1.0);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + 0.05*vec4(gs_in[0].norm, 0.0);
    EmitVertex();
    EndPrimitive();

    fragColor = vec3(1.0, 0.3, 0.3);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + 0.05*vec4(gs_in[0].tan, 0.0);
    EmitVertex();
    EndPrimitive();

    fragColor = vec3(0.3, 1.0, 0.3);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();
    gl_Position = gl_in[0].gl_Position + 0.05*vec4(gs_in[0].bTan, 0.0);
    EmitVertex();
    EndPrimitive();
}

#fragment_shader
#version 330 core

in vec3 fragColor;

out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}
