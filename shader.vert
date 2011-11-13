#version 330 core

in vec3 in_Position;
in vec3 in_Color;

out vec3 pass_Color;

void main(void)
{
     gl_Position = (vec4(in_Position, 1.0)/2)+vec4(0.0,0.0,1.0,1.0);
     pass_Color = in_Color;
}
