#version 330 core

in vec3 texCoords;
in vec3 norm;
in vec3 tan;
in vec3 bitan;
in vec3 light;
in vec3 eyevec;
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
        //tangent space lighting
        float lightAngle =max(dot(light,norm),0.0);//max(dot(light,vec3(0,0,1)),0.0);
        float diffuseLightFalloff = min((10.0/(lightDist*lightDist)),1.0);
        vec4 lightIntensity = diffuseLightFalloff * light.z * lightDiffuse + lightAmbient;
        
        vec4 colorTexel = texture(colorTex,texCoords.xy);
        FragColor = colorTexel*lightIntensity;//vec4(light.y,light.y,light.y,1);//colorTexel*
}
