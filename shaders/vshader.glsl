
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
// vertex ONLY ^^^

layout (location = 2) in vec3 translation;
layout (location = 3) in float scale;
// per instance ^^^

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 ourColor;

void main()
{
   vec3 worldPos = (aPos * scale) + translation;
   gl_Position = proj * view * model * vec4(worldPos, 1.0);
   ourColor = aColor;
}