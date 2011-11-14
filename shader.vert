#version 330 core

in vec3 in_Position;
in vec3 in_Color;
in vec3 in_Normal;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform float scalar;
uniform vec4 lightPosition;

out vec3 pass_Color;
out vec3 pass_Normal;
out vec4 pass_LightPos;

void main(void)
{
    gl_Position = (viewMatrix*vec4(in_Position[1],in_Position[2],-in_Position[0], 1.0)/scalar)+vec4(0.0,-0.4,1.0,1.0);
     pass_Color = in_Color;
     pass_Normal = (viewMatrix*modelMatrix*vec4(in_Normal[1],in_Normal[2],-in_Normal[0],1.0)).xyz;
     pass_LightPos = viewMatrix*modelMatrix*lightPosition;
}
