#version 330 core

layout(location = 0) out vec4 fragmentColor;

in vec4 normal;

void main(void)
{
    vec3 n = normalize(normal.xyz);
    // Map normal components from [-1, 1] to [0, 1] range for RGB colors
    vec3 normalColor = (n + vec3(1.0, 1.0, 1.0)) * 0.5;
    fragmentColor = vec4(normalColor, 1.0);
}
