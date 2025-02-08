#version 330 core
in vec3 pos;

uniform vec4 ourColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(pos, 1.0);
}