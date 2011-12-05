#version 330 core

in vec3 pass_TexCoords;
in vec3 pass_TanNormal;
in vec3 pass_ObjNormal;
in vec3 pass_Tangent;
in vec4 pass_LightPos;
in vec4 pass_Position;

out vec4 FragColor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
uniform sampler2D normTex;
uniform sampler2D specTex;

void main(void) {
	// calculate basic texture color
	vec4 colorTexel = texture(colorTex,pass_TexCoords.xy);
	
	//////////////////////////////////////
        //parallax mapping, doesn't work yet//
        //////////////////////////////////////
	float depth = texture(depthTex,pass_TexCoords.xy).x;
	vec3 bitan = cross(pass_TanNormal,pass_Tangent);
        vec3 eyevec = normalize(-(inverse(viewMatrix*modelMatrix)*pass_Position)).xyz;
        vec3 bipara = normalize(cross(pass_TanNormal,eyevec));
        vec3 paravec = normalize(cross(pass_TanNormal,bipara));
        float parallaxLength = -((sqrt(1-pass_TanNormal.z*pass_TanNormal.z))/tan(acos(dot(eyevec,paravec))))/50;
	vec2 parallaxOffset = parallaxLength*vec2(dot(-paravec,normalize(pass_Tangent)),dot(-paravec,normalize(cross(normalize(pass_Tangent),pass_TanNormal))));
	vec4 paraTexel = texture(colorTex,pass_TexCoords.xy+parallaxOffset);
	
	// diffuse lighting calculations
	float diffuseAngle = max(dot(pass_LightPos.xyz/length(pass_LightPos.xyz),pass_ObjNormal),0.0);
	float diffuseLightFalloff = (1.0/length(pass_LightPos)*length(pass_LightPos));
	vec4 lightIntensity = diffuseAngle * lightDiffuse * diffuseLightFalloff;
	// specular lighting
	lightIntensity += pow(diffuseAngle,8)*texture(specTex,pass_TexCoords.xy);
	// ambient lighting calculations
	lightIntensity += lightAmbient;
	// calculate the final fragment color
	FragColor = paraTexel*lightIntensity;
      //float depth = texture(depthTex,pass_TexCoords.xy).x;
      
}
