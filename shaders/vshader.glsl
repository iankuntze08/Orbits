
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
// layout (location = 2) in vec4 aOffset;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 translation;

out vec3 ourColor;

void main()
{
   gl_Position = proj * view * model * translation * vec4(aPos, 1.0);
   ourColor = aColor;
}