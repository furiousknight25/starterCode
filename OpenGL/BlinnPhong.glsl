#version 330 core

layout(location = 0) out vec4 fragmentColor;

uniform vec3 diffuseComponent;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float specularExponent;

in vec4 normal;
in vec4 lightDir;
in vec4 viewDir;

void main(void)
{
    vec3 n = normalize(normal.xyz);
    vec3 l = normalize(lightDir.xyz);
    vec3 v = normalize(viewDir.xyz);

    // Diffuse part (Lambertian)
    float val = max(0.0, dot(n, l));
    vec3 kd = diffuseComponent;
    if (length(kd) == 0.0) {
        kd = diffuseColor;
    }
    if (length(kd) == 0.0) {
        kd = vec3(0.9, 0.5, 0.2);
    }
    vec3 diffuseShading = vec3(kd.r * val, kd.g * val, kd.b * val);

    // Specular part (Blinn-Phong halfway vector)
    vec3 ks = specularColor;
    if (length(ks) == 0.0) {
        ks = vec3(1.0, 1.0, 1.0); // default white highlight
    }
    float p = (specularExponent > 0.0) ? specularExponent : 32.0;

    vec3 specularShading = vec3(0.0);
    if (val > 0.0) {
        vec3 h = normalize(l + v);
        float nDotH = max(0.0, dot(n, h));
        specularShading = ks * pow(nDotH, p);
    }

    vec3 ambient = 0.08 * kd;

    fragmentColor = vec4(ambient + diffuseShading + specularShading, 1.0);
}
