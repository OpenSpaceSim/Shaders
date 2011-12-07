#version 330 core

// Universal Values
in vec3 texCoords;
in vec3 eyevec;
out vec4 FragColor;

// Lighting Values
uniform vec4 lightAmbient;
uniform vec4 lightDiffuse;
in vec3 halfVec;
in vec3 light;
in float lightDist;

// our textures
uniform sampler2D colorTex;
uniform sampler2D depthTex;
uniform sampler2D normTex;
uniform sampler2D specTex;
uniform float samples;
uniform float depth;

// The actual tecture coord after parallax mapping
vec2 offsetCoord;

// Constants for use in parallax stuff

vec2 ComputeParallaxOcclusionOffset(float initOffsetX, float initOffsetY) {
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
        vec2 offsetCoord = texCoords.xy+vec2(initOffsetX*delta.x,initOffsetY*delta.y);
        
        while(dist > height) //step through the texture
        {
                height = (texture(depthTex,offsetCoord).x);
                offsetCoord += delta;
                dist -= inc;
        }
        return offsetCoord;
}
float ComputeParallaxOcclusionVisibility(vec2 texCoord, vec3 point, float pointDist) {
        /// parallax occlusion mapping
        ///
        /// here we calculate the parallax offset.
        /// at each pixel we check to make sure we don't intersect with a previous
        /// position on the height map
        float shadowSamples = samples/10;
        float texCoordDepth = texture2D(depthTex,texCoord).x * depth;
        float texCoordHorizUnit = depth*length(vec2(-eyevec.x, eyevec.y) * depth / (eyevec.z * shadowSamples))/(1.0/shadowSamples);
        float texCoordHoriz = texCoordHorizUnit*length(texCoord);
        vec3 pointVec = eyevec * sqrt(pow(texCoordHoriz,2)+texCoordDepth*texCoordDepth);
        vec3 lightVec = normalize(point*pointDist - pointVec);
        lightVec = vec3(-lightVec.x,-lightVec.y,lightVec.z);
        
        //amount to decrement the distance each step
        float inc = 1.0/shadowSamples;
        
        //amount to shift texture coordinates each step
        vec2 delta = vec2(-lightVec.x, lightVec.y) * depth / (lightVec.z * shadowSamples);
        
        //height of the eye vector
        float dist = texture2D(depthTex,texCoord).x;
        //height of current entry in texture map, starts at 0.0 and increases
        float height = 0.0;
        //texture coordinates
        vec2 offsetCoord = texCoord.xy;
        float max_diff = 0;
        while(dist < 1) //step through the texture
        {
                height = (texture(depthTex,offsetCoord).x);
                offsetCoord += delta;
                dist += inc;
                if (height-dist>max_diff) {
                        max_diff = height-dist;
                }
        }
        float shadowPass = 1;
        if (max_diff>0) {
                shadowPass = 1-sqrt(sqrt((max_diff/texture2D(depthTex,texCoord).x)));
        }
        return shadowPass;
}

mat4 ComputeAlias(float left, float right, float top, float bottom) {
	/* Computes the parallax offsets for in-texture anti-aliasing
	   Function supports 2-8x, currently configured for 4 */
        float hmid = (left + right)/2;
        float vmid = (top + bottom)/2;
        vec2 TL = ComputeParallaxOcclusionOffset(left, top);
        vec2 BR = ComputeParallaxOcclusionOffset(right, bottom);
        vec2 TR = ComputeParallaxOcclusionOffset(right, top);
        vec2 BL = ComputeParallaxOcclusionOffset(left, bottom);
/*	vec2 ML = ComputeParallaxOcclusionOffset(left, vmid);
        vec2 MR = ComputeParallaxOcclusionOffset(right, vmid);
        vec2 TM = ComputeParallaxOcclusionOffset(hmid, top);
        vec2 BM = ComputeParallaxOcclusionOffset(hmid, bottom);*/
        vec4 zero = vec4(0,0,0,0);
        return mat4(TL,BR,TR,BL,zero,zero);//,TM,ML,MR,BM);
}

vec4 ComputeTex(mat4 alias, sampler2D tex) {
	/* Uses a matrix of aliases to compute in-texture anti-aliasing
           Function supports 2-8x, currently configured for 4 */
        vec4 texel = vec4(0,0,0,0);
        vec4 row = alias[0];
        texel += texture2D(tex, vec2(row[0],row[1]));
        texel += texture2D(tex, vec2(row[2],row[3]));
        row = alias[1];
        texel += texture2D(tex, vec2(row[0],row[1]));
        texel += texture2D(tex, vec2(row[2],row[3]));
/*        row = alias[2];
        texel += texture2D(tex, vec2(row[0],row[1]));
        texel += texture2D(tex, vec2(row[2],row[3]));
        row = alias[3];
        texel += texture2D(tex, vec2(row[0],row[1]));
        texel += texture2D(tex, vec2(row[2],row[3]));*/
        return texel/4;
}

vec4 GetLightingFactor(vec3 point, float pointDist, vec4 pointColor) {
	// SHADOW MAPPING
	// Compute light's shadow factor
	float shadow = ComputeParallaxOcclusionVisibility(offsetCoord, point, pointDist);
	// NORMAL MAPPING
	vec3 surface_normal = normalize((texture2D(normTex, offsetCoord) * 2.0 - 1.0).xyz);
	// DIFFUSE LIGHTING
	// Quadradic light dispersal function
	float diffuseLightFalloff = min((10.0/(pointDist*pointDist)),1.0);
	// SPECULAR LIGHTING W SPECULAR MAPPING
	vec4 specularColor = texture(specTex,offsetCoord);
	float specularIntensity = length(specularColor);
	// PUTTING THE LIGHT TOGETHER
	return shadow*((diffuseLightFalloff * point.z * pointColor) + specularIntensity*specularColor * pow (max (dot (halfVec, surface_normal), 0.0), 2.0));
}

void main(void) {
        offsetCoord = ComputeParallaxOcclusionOffset(0,0);
        //mat4 alias8 = ComputeAlias(-10,10,-10,10);
        //offsetCoord now contains the location of of the correct texel after parallax occlusion adjustment
        
	// ambiant lighting
	vec4 lightIntensity = lightAmbient;
        // light sources
	lightIntensity += GetLightingFactor(light, lightDist, lightDiffuse);
        
        //get color from color map
        vec4 colorTexel = texture2D(colorTex,offsetCoord);
        //vec4 colorTexel = ComputeTex(alias8,colorTex);
        
        //calculate pixel color
        FragColor = colorTexel*lightIntensity;
}

