#version 330 core

in vec3 pass_Color;
in vec3 pass_Normal;
in vec4 pass_LightPos;

out vec4 FragColor;

void main(void)
{
      vec4 baseColor = vec4(pass_Color, 1.0);
      float phong = dot(pass_LightPos.xyz,pass_Normal);
      FragColor = baseColor*phong*phong;
}
