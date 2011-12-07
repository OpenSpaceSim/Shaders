#version 330 core

in vec3 texCoords;
in vec3 norm;
in vec3 tan;
in vec3 bitan;
in vec3 light;
in vec3 eyevec;
in vec3 halfVec;
in float lightDist;

uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
uniform sampler2D normTex;
uniform sampler2D specTex;

out vec4 FragColor;

void main(void)
{
	// ambiant lighting
	vec4 lightIntensity = lightAmbient;
        // diffuse lighting
	vec3 surface_normal = normalize((texture2D(normTex, texCoords.xy) * 2.0 - 1.0).xyz+norm);
	//surface_normal = norm;
        float lightAngle =max(dot(light,surface_normal),0.0000001);//max(dot(light,vec3(0,0,1)),0.0);
        float diffuseLightFalloff = min((10.0/(lightDist*lightDist)),1.0);
        lightIntensity += diffuseLightFalloff * light.z * lightDiffuse;
	// specular lighting
	vec4 specularColor = texture(specTex,texCoords.xy);
	float specularIntensity = length(specularColor);
	lightIntensity += specularColor * pow (max (dot (halfVec, surface_normal), 0.0), 2.0);
	//lightIntensity += specularColor*specularIntensity*pow(lightAngle,specularIntensity);
        
        vec4 colorTexel = texture(colorTex,texCoords.xy);
        FragColor = colorTexel*lightIntensity;//vec4(light.y,light.y,light.y,1);//colorTexel*
}
/*
#version 330 core

in vec3 pass_TexCoords;
in vec3 pass_TanNormal;
in vec3 pass_ObjNormal;
in vec3 pass_Tangent;
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
	vec4 paraTexel = texture(colorTex,pass_TexCoords.xy);//+parallaxOffset);
	
	//////////////////////////////////////
        //Lighting Calculations             //
        //////////////////////////////////////
	mat4 modelviewMatrix = viewMatrix*modelMatrix;
	// Normal mapping
	vec3 norm = (texture2D(normTex, pass_TexCoords.xy) * 2.0 - 1.0).xyz;
	norm += pass_TanNormal; // transform the tangent normal by the normal map
	norm = normalize(vec3(norm[1],norm[2],-norm[0]));
	norm = (modelviewMatrix*vec4(norm,1.0)).xyz; // put in modelview space
	// Calculate the diffuse lighting Angle
	vec3 diffLightVec = normalize(pass_LightPos.xyz);
	float diffuseAngle = max(0.0, (dot(diffLightVec,norm)));
	// Dim the light by a squared function 
	float diffuseLightFalloff = (3.0/length(pass_LightPos)*length(pass_LightPos));
	// Calculate the diffuse lighting
	vec4 lightIntensity = diffuseAngle * lightDiffuse * diffuseLightFalloff;
	// specular lighting
	float SpecularIntensity = texture(specTex,pass_TexCoords.xy).x;
	lightIntensity += SpecularIntensity*pow(diffuseAngle,SpecularIntensity);
	// ambient lighting calculations
	lightIntensity += lightAmbient;
	// calculate the final fragment color
	FragColor = paraTexel*lightIntensity;
      
}
*/
