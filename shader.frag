#version 330 core

in vec3 pass_Color;

out vec4 FragColor;

void main(void)
{
      FragColor = vec4(pass_Color, 1.0);
}
