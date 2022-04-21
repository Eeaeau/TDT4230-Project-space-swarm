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
vec3 ambient= vec3(0.1); 

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

//	Classic Perlin 3D Noise 
//	by Stefan Gustavson
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
vec3 fade(vec3 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

float cnoise(vec3 P){
  vec3 Pi0 = floor(P); // Integer part for indexing
  vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
  Pi0 = mod(Pi0, 289.0);
  Pi1 = mod(Pi1, 289.0);
  vec3 Pf0 = fract(P); // Fractional part for interpolation
  vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = Pi0.zzzz;
  vec4 iz1 = Pi1.zzzz;

  vec4 ixy = permute(permute(ix) + iy);
  vec4 ixy0 = permute(ixy + iz0);
  vec4 ixy1 = permute(ixy + iz1);

  vec4 gx0 = ixy0 / 7.0;
  vec4 gy0 = fract(floor(gx0) / 7.0) - 0.5;
  gx0 = fract(gx0);
  vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
  vec4 sz0 = step(gz0, vec4(0.0));
  gx0 -= sz0 * (step(0.0, gx0) - 0.5);
  gy0 -= sz0 * (step(0.0, gy0) - 0.5);

  vec4 gx1 = ixy1 / 7.0;
  vec4 gy1 = fract(floor(gx1) / 7.0) - 0.5;
  gx1 = fract(gx1);
  vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
  vec4 sz1 = step(gz1, vec4(0.0));
  gx1 -= sz1 * (step(0.0, gx1) - 0.5);
  gy1 -= sz1 * (step(0.0, gy1) - 0.5);

  vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
  vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
  vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
  vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
  vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
  vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
  vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
  vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

  vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.x;
  g010 *= norm0.y;
  g100 *= norm0.z;
  g110 *= norm0.w;
  vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.x;
  g011 *= norm1.y;
  g101 *= norm1.z;
  g111 *= norm1.w;

  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
  float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
  float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
  float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
  float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
  float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
  float n111 = dot(g111, Pf1);

  vec3 fade_xyz = fade(Pf0);
  vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
  vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
  float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
  return 2.2 * n_xyz;
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
		brightColor.rgb += 6*max(fresnelSchlick(dot(viewDir, normal), vec3(0.02)) * vec3(0.5,0.5,1), 0) * (cnoise(vec3(8*texcoord, gameTime/2))+0.1);
	}

//	float lum = max(dot(normal, normalize(sun_position)), 0.0);
//	texture(normalTexture, texcoord).rgb
	float lum = 1.0;
	if (selfShadow == 1) {
		lum = max(dot(normal, normalize(sun_position)), 0.0);
	}

	fragColor = diffuseColor * vec4((ambient + lum) * sun_color, 1.0);

//	fragColor = vec4(vec3(perlin(10*texcoord, 2, gameTime_us/10)), 1.0);
//	fragColor = vec4(vec3(cnoise(vec3(10*texcoord, gameTime/2))), 1.0);
//	fragColor = vec4(normalize(fragPos), 1.0);

//	fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
//	fragColor.rgb = normal;



}