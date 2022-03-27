#version 450 core

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 4) vec3 fragPos;
//in layout(location = 5) mat3 TBN;
in layout(location = 9) vec3 tangent_in;

struct Texture {
    sampler2D diffuse;
    sampler2D normal;
};  

//uniform Texture texture_in;

//in layout(location = 6) uniform Texture texture_in;
//
//layout(binding = 1) uniform sampler2D diffuseTexture; // was not happy using implicit uniform
//layout(binding = 2) uniform sampler2D normalTexture;
//layout(binding = 3) uniform sampler2D roughnessTexture;

uniform vec3 viewPos;
uniform vec3 ballPos;
uniform mat3 normalMatrix;

out vec4 color;


float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

vec3 currentLightPos = vec3(0, 0, 0);
vec3 result = vec3(0);

float ambientStrenght = 0.0;
vec3 ambientColor = vec3(1, 1, 1);

vec3 diffuseColor = vec3(1);
vec3 emissionColor = vec3(0.0);

float specularStrength = 0.15;

vec3 lightColor = vec3(4);


void main()
{
    vec3 normal = normalize(normal_in);
    vec3 lightDir = normalize(vec3(1,1,-.5));
    color = vec4(result, 1.0);
}