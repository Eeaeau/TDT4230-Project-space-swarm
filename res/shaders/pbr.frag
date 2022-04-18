#version 450 core

in vec3 normal;
in vec3 tangent;
in vec3 position;
in vec2 texcoord;
in mat3 TBN;

//layout(location = 3) in uniform vec3 emissiveFactor;
uniform vec3 emissiveFactor;
uniform int useNormalTexture;
uniform int useDiffuseTexture;
uniform int useRoughnessTexture;
uniform int useEmissiveTexture;
//uniform float roughnessFactor;

//uniform sampler2D tex;
layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D normalTexture;
layout(binding = 2) uniform sampler2D roughnessTexture;

vec3 sun_position = vec3(-1, 1, -1); 
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
vec4 diffuseColor = vec4(1);

void main() {

	vec3 normal = normalize(normal);
	if (useNormalTexture == 1) {
		normal = texture(normalTexture, texcoord).rgb*2-1;
		normal = normalize(TBN * normal);
	}

	if (useDiffuseTexture == 1) {
		diffuseColor = texture(diffuseTexture, texcoord);
	}
	if (useEmissiveTexture == 1) {
		emissiveColor = texture(diffuseTexture, texcoord);
	}


//	float lum = max(dot(normal, normalize(sun_position)), 0.0);
//	texture(normalTexture, texcoord).rgb
	float lum = max(dot(normal, normalize(sun_position)), 0.0);
	fragColor = diffuseColor * vec4((ambient + lum) * sun_color, 1.0);
//	fragColor.rgb = normal;

	brightColor vec4(vec3(0), 1);

    if (fragBrightness(emissiveColor) > 0.3f) {
		brightColor.rgb += emissiveColor.rgb;
	}

	brightColor.rgb *= emissiveFactor;

}