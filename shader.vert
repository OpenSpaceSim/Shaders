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
out vec3 halfVec;
out float lightDist;

void main(void) {
	mat4 modelviewMatrix = viewMatrix * modelMatrix;
	gl_Position = (modelviewMatrix*vec4(in_Position, 1.0)/scalar)+vec4(0.0,-0.25,1.0,1.0);
	
	//pass_ObjNormal = (modelMatrix*vec4(in_Normal[1],in_Normal[2],-in_Normal[0],1.0)).xyz;
	
	//pass_Position = gl_Position;
	//pass_LightPos = lightPosition-gl_Position;
	
	texCoords = in_TexCoords;
	mat4 normalMatrix = transpose(inverse(modelMatrix));
	norm = normalize((normalMatrix*vec4(in_Normal,1)).xyz);
        tan = normalize((normalMatrix*vec4(in_Tangent,1)).xyz);
        bitan = cross(norm,tan);
        mat3 tbnMatrix = mat3(tan.x, bitan.x, norm.x,
                         tan.y, bitan.y, norm.y,
                         tan.z, bitan.z, norm.z);
        mat3 itbnMatrix = inverse(tbnMatrix);
        light = normalize(tbnMatrix*(lightPosition-gl_Position).xyz);
        eyevec = normalize(tbnMatrix * -gl_Position.xyz);
        lightDist = length(((viewMatrix*lightPosition)-gl_Position).xyz);
	
	halfVec = normalize(gl_Position.xyz + light);
	halfVec = vec3(dot(halfVec,tan),dot(halfVec,bitan),dot(halfVec,norm));
}
