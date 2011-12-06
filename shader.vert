#version 330 core

in vec3 in_Position;
in vec3 in_TexCoords;
in vec3 in_Normal;
in vec3 in_Tangent;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform float scalar;
uniform vec4 lightPosition;

out vec3 texCoords;
out vec3 norm;
out vec3 tan;
out vec3 bitan;
out vec3 light;
out vec3 eyevec;
out float lightDist;

void main(void) {
	mat4 modelviewMatrix = viewMatrix * modelMatrix;
	gl_Position = (modelviewMatrix*vec4(in_Position[1],in_Position[2],-in_Position[0], 1.0)/scalar)+vec4(0.0,-0.25,1.0,1.0);
	pass_TexCoords = in_TexCoords;
	pass_ObjNormal = (modelMatrix*vec4(in_Normal[1],in_Normal[2],-in_Normal[0],1.0)).xyz;
	pass_TanNormal = in_Normal;
	vec4 absPosition = inverse(modelviewMatrix)*gl_Position;
	pass_Position = absPosition;
	pass_LightPos = lightPosition-absPosition;
        pass_Tangent = in_Tangent;
}
