#version 330 core

in vec3 pass_TexCoords;
in vec3 pass_TanNormal;
in vec3 pass_ObjNormal;
//in vec3 pass_Tangent;
in vec4 pass_LightPos;
in vec4 pass_Position;

out vec4 FragColor;

uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
uniform sampler2D normTex;
uniform sampler2D specTex;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;

const float PI = 3.14159265358979323;
void main(void) {
	// calculate basic texture color
	vec4 colorTexel = texture(colorTex,pass_TexCoords.xy);
	// diffuse lighting calculations
	//vec3 normal = normalize(pass_ObjNormal);//normalize()*.5+pass_ObjNormal*.5;
	// normal mapping
	//vec3 norm = (texture2D(normTex, pass_TexCoords.xy) * 2.0 - 1.0).xyz;
	vec3 norm = (viewMatrix*vec4(pass_ObjNormal,1.0)).xyz;
	vec3 diffLightVec = normalize(pass_LightPos.xyz);
	float diffuseAngle = max(0.0, (dot(diffLightVec,norm)));
	// back to sanity
	//float diffuseAngle = max(dot(diffLightVec,normal),0.0);
	float diffuseLightFalloff = (1.0/length(pass_LightPos)*length(pass_LightPos));
	vec4 lightIntensity = diffuseAngle * lightDiffuse * diffuseLightFalloff*2;
	// specular lighting
	lightIntensity += 5*pow(diffuseAngle,8)*texture(specTex,pass_TexCoords.xy);
	// ambient lighting calculations
	lightIntensity += lightAmbient;
	// calculate the final fragment color
	FragColor = colorTexel*lightIntensity;
      //float depth = texture(depthTex,pass_TexCoords.xy).x;
      
      //////////////////////////////////////
      //parallax mapping, doesn't work yet//
      //////////////////////////////////////
      //vec3 eyevec = normalize(1.0-pass_Position.xyz);
      /*vec3 bipara = cross(normalize(pass_Normal),eyevec);
      vec3 paravec = normalize(cross(pass_Normal,bipara));
      float parallaxLength = ((sqrt(1-pass_Normal.z*pass_Normal.z))/tan(acos(dot(eyevec,paravec))))/1000;
      vec2 parallaxOffset = parallaxLength*vec2(dot(-paravec,normalize(pass_Tangent)),dot(-paravec,normalize(cross(normalize(pass_Tangent),pass_Normal))));
      vec4 paraColor = texture(colorTex,pass_TexCoords.xy+parallaxOffset);
      //FragColor = vec4(parallaxOffset.x,parallaxOffset.y,parallaxLength,0);
      //FragColor = vec4(pass_Tangent,0);
      //vec4 paraColor = 200*vec4(parallaxOffset,parallaxOffset);
	*/
	/*float fBumpScale = 0.05f;
	vec2 vHalfOffset = eyevec.xy * (depth) * fBumpScale;
	depth = (depth + texture(depthTex, pass_TexCoords.xy + vHalfOffset).x) * 0.5;
	vHalfOffset = eyevec.xy * (depth) * fBumpScale;
	depth = (depth + texture(depthTex, pass_TexCoords.xy + vHalfOffset).x) * 0.5;
	vHalfOffset = eyevec.xy * (depth) * fBumpScale;
	vec4 paraColor = texture(colorTex,pass_TexCoords.xy + vHalfOffset);
	
	float angle = max(dot(pass_LightPos.xyz/length(pass_LightPos.xyz),pass_Normal),0.0);
      vec4 intensity = lightAmbient + angle*lightDiffuse + pow(angle,8)*lightSpecular;
      FragColor = paraColor;//*intensity*(1.0/length(pass_LightPos));*/
}
