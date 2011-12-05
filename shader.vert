#version 330 core

in vec3 in_Position;
in vec3 in_TexCoords;
in vec3 in_Normal;
in vec3 in_Tangent;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform float scalar;
uniform vec4 lightPosition;

out vec3 pass_TexCoords;
out vec3 pass_TanNormal;
out vec3 pass_ObjNormal;
out vec4 pass_LightPos;
out vec4 pass_Position;
out vec3 pass_Tangent;

void main(void) {
	mat4 modelviewMatrix = viewMatrix * modelMatrix;
	gl_Position = (modelviewMatrix*vec4(in_Position[1],in_Position[2]+4,-in_Position[0], 1.0)/scalar)+vec4(0.0,-0.25,1.0,1.0);
	pass_TexCoords = in_TexCoords;
	pass_ObjNormal = (modelMatrix*vec4(in_Normal[1],in_Normal[2],-in_Normal[0],1.0)).xyz;
	pass_TanNormal = in_Normal;
	pass_Position = gl_Position;
	pass_LightPos = lightPosition-gl_Position;
        pass_Tangent = in_Tangent;
}
