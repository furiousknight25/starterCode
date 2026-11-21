#version 330 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;

uniform vec4 lightPosWorld;
uniform vec4 eyePosWorld;

out vec4 normal;
out vec4 lightDir;
out vec4 viewDir;

void main(void)
{
    // Transform normal into world space
    mat4 normMat = normalMatrix;
    if (normMat[0][0] == 0.0 && normMat[1][1] == 0.0 && normMat[2][2] == 0.0) {
        normMat = transpose(inverse(modelMatrix));
    }
    normal = normalize(normMat * vec4(in_Normal, 0.0));

    // Transform vertex position into world space
    vec4 vertexWorldPos = modelMatrix * vec4(in_Position, 1.0);

    // Direction from vertex to light in world space
    vec4 lPos = lightPosWorld;
    if (lPos.w == 0.0 && length(lPos.xyz) == 0.0) {
        lPos = vec4(0.0, 5.0, 5.0, 1.0);
    }
    lightDir = normalize(lPos - vertexWorldPos);

    // Direction from vertex to eye / camera in world space (for Blinn-Phong specular)
    vec4 ePos = eyePosWorld;
    if (ePos.w == 0.0 && length(ePos.xyz) == 0.0) {
        ePos = vec4(0.0, 0.0, 3.0, 1.0);
    }
    viewDir = normalize(ePos - vertexWorldPos);

    // Output clip space position
    gl_Position = projMatrix * viewMatrix * vertexWorldPos;
}
