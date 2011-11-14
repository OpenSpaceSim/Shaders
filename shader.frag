#version 330 core

in vec3 pass_Color;
in vec3 pass_Normal;
in vec4 pass_LightPos;

out vec4 FragColor;

uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;

void main(void)
{
      vec4 baseColor = vec4(0.4,0.4,0.4, 1.0);
      float angle = max(dot(pass_LightPos.xyz,pass_Normal),0.0);
      vec4 intensity = lightAmbient + angle*lightDiffuse + pow(angle,8)*lightSpecular;
      FragColor = baseColor*intensity*(1.0/length(pass_LightPos));
}
