#version 330 core
in vec3 color;
layout(location=0) out vec4 fragmentColor; //tstandard window framebuffer only has color attachment 0.
//when did we ever set location 0 to be fragment color I thought it was just set for the vertex shader

void main(void) {
    fragmentColor =  vec4(color.x, color.y,color.z, 1);
}
