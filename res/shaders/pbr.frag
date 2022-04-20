#version 450 core

in vec3 normal;
in vec3 tangent;
in vec3 position;
in vec2 texcoord;
in mat3 TBN;

//layout(location = 3) in uniform vec3 emissiveFactor;
uniform vec4 baseColorFactor;
uniform vec3 emissiveFactor;
uniform vec3 cameraFaceDirection;
uniform int useNormalTexture;
uniform int useDiffuseTexture;
uniform int useRoughnessTexture;
uniform int useEmissiveTexture;
//uniform float roughnessFactor;
uniform int selfShadow;
uniform int useFresnel;

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

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
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


//	float lum = max(dot(normal, normalize(sun_position)), 0.0);
//	texture(normalTexture, texcoord).rgb
	float lum = 1.0;
	if (selfShadow == 1) {
		lum = max(dot(normal, normalize(sun_position)), 0.0);
	}

	fragColor = diffuseColor * vec4((ambient + lum) * sun_color, 1.0);

	fragColor = vec4(normal, 1.0);

//	fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
//	fragColor.rgb = normal;

	brightColor vec4(vec3(0), 1);

//    if (fragBrightness(emissiveColor) > 0.3f) {
//		
//	}

	brightColor.rgb = hillEquation(fragBrightness(emissiveColor), 0.5, 4) * emissiveColor.rgb;

	brightColor.rgb *= emissiveFactor;

	if (useFresnel == 1) {
//		brightColor.rgb += 2 * fresnelSchlick(dot(-cameraFaceDirection, normal), vec3(0.3)) * vec3(0.5,0.5,1);
//		brightColor.rgb += vec3(0.5,0.5,1);
	}

}