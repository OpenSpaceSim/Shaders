#version 330 core

in vec3 pass_TexCoords;
in vec3 pass_Normal;
in vec3 pass_Tangent;
in vec4 pass_LightPos;
in vec4 pass_Position;

out vec4 FragColor;

uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform sampler2D colorTex;
uniform sampler2D depthTex;

void main(void)
{
      //calculate basic texture color
      vec4 baseColor = texture(colorTex,pass_TexCoords.xy);
      float depth = texture(depthTex,pass_TexCoords.xy).x;
      
      //////////////////////////////////////
      //parallax mapping, doesn't work yet//
      //////////////////////////////////////
      vec3 eyevec = normalize(1.0-pass_Position.xyz);
      /*vec3 bipara = cross(normalize(pass_Normal),eyevec);
      vec3 paravec = normalize(cross(pass_Normal,bipara));
      float parallaxLength = ((sqrt(1-pass_Normal.z*pass_Normal.z))/tan(acos(dot(eyevec,paravec))))/1000;
      vec2 parallaxOffset = parallaxLength*vec2(dot(-paravec,normalize(pass_Tangent)),dot(-paravec,normalize(cross(normalize(pass_Tangent),pass_Normal))));
      vec4 paraColor = texture(colorTex,pass_TexCoords.xy+parallaxOffset);
      //FragColor = vec4(parallaxOffset.x,parallaxOffset.y,parallaxLength,0);
      //FragColor = vec4(pass_Tangent,0);
      //vec4 paraColor = 200*vec4(parallaxOffset,parallaxOffset);
	*/
	float fBumpScale = 0.05f;
	vec2 vHalfOffset = eyevec.xy * (depth) * fBumpScale;
	depth = (depth + texture(depthTex, pass_TexCoords.xy + vHalfOffset).x) * 0.5;
	vHalfOffset = eyevec.xy * (depth) * fBumpScale;
	depth = (depth + texture(depthTex, pass_TexCoords.xy + vHalfOffset).x) * 0.5;
	vHalfOffset = eyevec.xy * (depth) * fBumpScale;
	vec4 paraColor = texture(colorTex,pass_TexCoords.xy + vHalfOffset);
	
	float angle = max(dot(pass_LightPos.xyz/length(pass_LightPos.xyz),pass_Normal),0.0);
      vec4 intensity = lightAmbient + angle*lightDiffuse + pow(angle,8)*lightSpecular;
      FragColor = paraColor*intensity*(1.0/length(pass_LightPos));
}
