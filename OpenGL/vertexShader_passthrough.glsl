#version 330 core
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Color;

out vec3 color; //sends this to rasterizer
//EVERY SING FREAKING DAMN THING THAT GOES THROUGH OUT GETS MULTIPLIED BY LE BARYCENTRIC SHIT

void main(void)
{
    gl_Position = vec4(in_Position, 1.0); //gl_Position is a global output of opengl
    color = in_Color;//where does this even show it get turned into barycentric cords?
}