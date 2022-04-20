#version 450 core

in vec3 normal;
in vec3 tangent;
in vec3 position;
in vec3 fragPos;
in vec2 texcoord;
in mat3 TBN;

#define M_PI 3.14159265358979323846

//layout(location = 3) in uniform vec3 emissiveFactor;
uniform vec4 baseColorFactor;
uniform vec3 emissiveFactor;
uniform vec3 cameraFaceDirection;
uniform vec3 viewPos;
uniform int useNormalTexture;
uniform int useDiffuseTexture;
uniform int useRoughnessTexture;
uniform int useEmissiveTexture;
//uniform float roughnessFactor;
uniform int selfShadow;
uniform int useFresnel;
uniform float gameTime;

//uniform sampler2D tex;
layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D normalTexture;
layout(binding = 2) uniform sampler2D roughnessTexture;
layout(binding = 3) uniform sampler2D emissiveTexture;

vec3 sun_position = vec3(-3, -0.5, -1); 
vec3 sun_color = vec3(2);
vec3 ambient= vec3(0.01); 

//out vec4 fragColor;
layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec4 brightColor;

float fragBrightness(vec4 fragColor) {
	float average = dot(fragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    return average;
}

vec4 emissiveColor = vec4(1);
vec4 diffuseColor = baseColorFactor;

float hillEquation(float L, float Ka, float n) {
	return 1/(1+pow(Ka/L,n));	
}

float rand(vec2 co){return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);}
float rand (vec2 co, float l) {return rand(vec2(rand(co), l));}
float rand (vec2 co, float l, float t) {return rand(vec2(rand(co, l), t));}

float perlin(vec2 p, float dim, float time) {
	vec2 pos = floor(p * dim);
	vec2 posx = pos + vec2(1.0, 0.0);
	vec2 posy = pos + vec2(0.0, 1.0);
	vec2 posxy = pos + vec2(1.0);
	
	float c = rand(pos, dim, time);
	float cx = rand(posx, dim, time);
	float cy = rand(posy, dim, time);
	float cxy = rand(posxy, dim, time);
	
	vec2 d = fract(p * dim);
	d = -0.5 * cos(d * M_PI) + 0.5;
	
	float ccx = mix(c, cx, d.x);
	float cycxy = mix(cy, cxy, d.x);
	float center = mix(ccx, cycxy, d.y);
	
	return center * 2.0 - 1.0;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {

	vec3 normal = normalize(normal);
	if (useNormalTexture == 1) {
		normal = texture(normalTexture, texcoord).rgb*2-1;
		normal = normalize(TBN * normal);
	}

	if (useDiffuseTexture == 1) {
		diffuseColor = texture(diffuseTexture, texcoord);
		emissiveColor = texture(diffuseTexture, texcoord);
	}

	if (useEmissiveTexture == 1) {
		emissiveColor = texture(emissiveTexture, texcoord);
	}


	vec3 cameraVec = viewPos - fragPos;
	vec3 viewDir = normalize(cameraVec);


	brightColor = vec4(vec3(0), 1);

	brightColor.rgb = hillEquation(fragBrightness(emissiveColor), 0.5, 4) * emissiveColor.rgb;

	brightColor.rgb *= emissiveFactor;

	if (useFresnel == 1) {
//		brightColor.rgb += 2*max(fresnelSchlick(dot(viewDir, normal), vec3(0.03)) * vec3(0.5,0.5,1), 0) * perlin(texcoord, 2, gameTime);
	}

//	float lum = max(dot(normal, normalize(sun_position)), 0.0);
//	texture(normalTexture, texcoord).rgb
	float lum = 1.0;
	if (selfShadow == 1) {
		lum = max(dot(normal, normalize(sun_position)), 0.0);
	}

	fragColor = diffuseColor * vec4((ambient + lum) * sun_color, 1.0);

	fragColor = vec4(vec3(perlin(10*texcoord, 2, gameTime/10000)), 1.0);
//	fragColor = vec4(normalize(fragPos), 1.0);

//	fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
//	fragColor.rgb = normal;



}