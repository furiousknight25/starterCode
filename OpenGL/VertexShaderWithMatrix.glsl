#version 330 core
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Color;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 in_Position; //sends this to rasterizer
out vec3 in_Normal;
//EVERY SING FREAKING DAMN THING THAT GOES THROUGH OUT GETS MULTIPLIED BY LE BARYCENTRIC SHIT

void main(void)
{
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0); //gl_Position is a global output of opengl
    pos = in_Color;//where does this even show it get turned into barycentric cords?
}