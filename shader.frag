#version 330 core

in vec3 texCoords;
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

const float samples = 1000;
const float depth = 0.3;

void main(void)
{
        /// parallax occlusion mapping
        ///
        /// here we calculate the parallax offset.
        /// at each pixel we check to make sure we don't intersect with a previous
        /// position on the height map
        
        //amount to decrement the distance each step
        float inc = 1.0/samples;
        
        //amount to shift texture coordinates each step
        vec2 delta = vec2(-eyevec.x, eyevec.y) * depth / (eyevec.z * samples);
        
        //height of the eye vector
        float dist = 1.0;
        //height of current entry in texture map, starts at 0.0 and increases
        float height = 0.0;
        //texture coordinates
        vec2 offsetCoord = texCoords.xy;
        
        while(dist > height) //step through the texture
        {
                height = (texture(depthTex,offsetCoord).x);
                offsetCoord += delta;
                dist -= inc;
        }
        //offsetCoord now contains the location of of the correct texel after parallax occlusion adjustment
        
	// ambiant lighting
	vec4 lightIntensity = lightAmbient;
	
        // diffuse lighting
        //get normal from normal map
	vec3 surface_normal = normalize((texture2D(normTex, offsetCoord.xy) * 2.0 - 1.0).xyz);
	//calcuale light variation due to normal
        float lightAngle =max(dot(vec3(0.0,0.0,1.0),surface_normal),0.0000001);
        //quadratic light falloff
        float diffuseLightFalloff = min((10.0/(lightDist*lightDist)),1.0);
        lightIntensity += diffuseLightFalloff * light.z * lightDiffuse;
        
	// specular lighting
	vec4 specularColor = texture(specTex,offsetCoord.xy);
	float specularIntensity = length(specularColor);
	lightIntensity += specularColor * pow (max (dot (halfVec, surface_normal), 0.0), 2.0);
        
        //get color from color map
        vec4 colorTexel = texture(colorTex,offsetCoord.xy);
        
        //calculate pixel color
        FragColor = colorTexel*lightIntensity;
}

