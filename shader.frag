#version 330 core

in vec2 pass_TexCoords;
in vec3 pass_Normal;
in vec4 pass_LightPos;
in vec4 pass_Position;

out vec4 FragColor;

uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform sampler2D colorTex;

void main(void)
{
      //calculate basic texture color
      vec4 baseColor = texture(colorTex,pass_TexCoords);
      
      //////////////////////////////////////
      //parallax mapping, doesn't work yet//
      //////////////////////////////////////
      //calculate normal map from color map
      float avg = (1.0-(baseColor.r + baseColor.g + baseColor.b)/3.0)/50.0;
      vec3 normvec = vec3(avg,avg,avg);
      vec3 eyevec = normalize(1.0 - pass_Position.xyz);
      vec3 bipara = cross(normvec,eyevec);
      vec3 paravec = normalize(cross(pass_Normal,bipara));
      float parallaxLength = avg/tan(acos(dot(eyevec,paravec)));
      vec2 parallaxOffset = vec2(parallaxLength/sqrt(2.0),parallaxLength/sqrt(2.0));
      vec4 paraColor = texture(colorTex,pass_TexCoords-parallaxOffset);
      
      
      float angle = max(dot(pass_LightPos.xyz/length(pass_LightPos.xyz),pass_Normal),0.0);
      vec4 intensity = lightAmbient + angle*lightDiffuse + pow(angle,8)*lightSpecular;
      FragColor = baseColor*intensity*(1.0/length(pass_LightPos));
}
