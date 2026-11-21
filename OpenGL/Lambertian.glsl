#version 330 core

layout(location = 0) out vec4 fragmentColor;

uniform vec3 diffuseComponent;
uniform vec3 diffuseColor;

in vec4 normal;
in vec4 lightDir;

void main(void)
{
    vec3 n = normalize(normal.xyz);
    vec3 l = normalize(lightDir.xyz);

    float val = max(0.0, dot(n, l));

    vec3 kd = diffuseComponent;
    if (length(kd) == 0.0) {
        kd = diffuseColor;
    }
    if (length(kd) == 0.0) {
        kd = vec3(0.9, 0.5, 0.2); // nice warm orange
    }

    vec3 diffuseShading = vec3(kd.r * val, kd.g * val, kd.b * val);

    // Small ambient so the unlit side doesn't disappear completely
    vec3 ambient = 0.08 * kd;

    fragmentColor = vec4(ambient + diffuseShading, 1.0);
}
